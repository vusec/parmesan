/*
 * Based on IDAssigner by Elia Geretto
 *
 * */
 
#include "parmesan/IDAssigner.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/CFG.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/raw_ostream.h"

#include<set>
#include<list>

#include <fstream>
#include <limits>
#include <sstream>

#define DEBUG_TYPE "idassign"

using namespace parmesan;
using namespace llvm;

STATISTIC(NumIDs, "The # of IDs generated");

static cl::opt<bool> ClEmitInfo(
    "idassign-emit-info",
    cl::desc("Write the debug info associated with the IDs to a file"),
    cl::init(false), cl::Hidden);

static cl::opt<std::string> ClInfoFile(
    "idassign-info-file",
    cl::desc("File that will contain the debug information in CSV format"),
    cl::init("-"), cl::Hidden);

static cl::opt<bool> ClEmitCfg(
    "idassign-emit-cfg",
    cl::desc("Write the static CFG to a file."),
    cl::init(false), cl::Hidden);

static cl::opt<std::string> ClCfgFile(
    "idassign-cfg-file",
    cl::desc("File that will contain the cfg information in CSV format"),
    cl::init("-"), cl::Hidden);

static cl::opt<bool> ClFollowIndDominators(
    "parmesan-follow-dominators",
    cl::desc("Collect all indirect call dominators, rather than just the closest"),
    cl::init(false), cl::Hidden);

static cl::opt<std::string> ClCustomTargetsFile(
    "custom-targets-file",
    cl::desc("Input file containing custom target lines of code."),
    cl::Hidden);
    //cl::init("-"), cl::Hidden);





class IDAssigner::IDGenerator {
  IdentifierType SerialIdentifier = 1;

public:
  IdentifierType getUniqueIdentifier() {
    assert(SerialIdentifier < std::numeric_limits<IdentifierType>::max());

    NumIDs++;
    return SerialIdentifier++;
  }
};

char IDAssigner::ID = 0;
IDAssigner::IDAssigner() : ModulePass(ID) {}
IDAssigner::~IDAssigner() = default;
std::set<std::tuple<IDAssigner::IdentifierType, IDAssigner::IdentifierType>> CfgEdges;
std::set<std::string> TraceFunctions = {"__angora_trace_cmp", "__angora_trace_switch", "__dfsw___angora_trace_cmp_tt", "__dfsw___angora_trace_exploit_val_tt", "__dfsw___angora_trace_switch_tt", "__dfsw___angora_trace_fn_tt"};

Type *VoidTy;
IntegerType *Int32Ty;
FunctionType *ParmeSanDummyCallTy;
Constant *ParmeSanDummyCall;

void IDAssigner::collectCallSiteDominators(Function *F) {
    for (auto &BB : *F) {
      CallSiteIdType PrevCallSiteId;
      for (auto &I : BB) {
          if (StoreInst *SI = dyn_cast<StoreInst>(&I)) {
              Value *V = SI->getPointerOperand();
              if (V) {
                StringRef name = V->getName();
                if (name == "__angora_indirect_call_site") {
                    Value *CV = SI->getValueOperand();
                    if (CV) {
                        if (ConstantInt *ConstInt = dyn_cast<ConstantInt>(CV)) {
                            PrevCallSiteId = ConstInt->getZExtValue();
                        }
                    }
                }
              }
          }
          else if (CallInst *CI = dyn_cast<CallInst>(&I)) {
              // Check if indirect call
              if (CI->getCalledFunction() == NULL) {
                  SmallPtrSet<Instruction *, 16> BranchSet;
                  SmallPtrSet<Instruction *, 16> Seen;
                  if (CI->getNumArgOperands() < 1)
                      continue;
                  auto ArgI = CI->getArgOperand(0);
                  if (!ArgI)
                      continue;
                  Instruction *Inst = dyn_cast<Instruction>(ArgI);
                  if (!Inst)
                      continue;
                  collectPreviousIndirectBranch(Inst, &BranchSet, &Seen);
                  std::set<CmpIdType> CSDominatorCmpIds;
                  for (auto E: BranchSet) {
                    BasicBlock *DomBB = E->getParent();
                    if (DomBB) {
                        IDAssigner::CmpIdType angoraCmpId = getAngoraCmpIdForBB(DomBB);
                        CSDominatorCmpIds.insert(angoraCmpId);
                    }

                  }
                  CallSiteDominatorsMap[PrevCallSiteId] = CSDominatorCmpIds; 
                  
              }
          }
      }
    }
}

IDAssigner::CmpIdType IDAssigner::getAngoraCmpIdForBB(BasicBlock *BB) {
    auto id = IdMap[BB];
    auto result = IdToAngoraMap[id];
    return result;
}

bool IDAssigner::runOnModule(Module &M) {
  IdentifierGenerator = make_unique<IDGenerator>();


  std::set<IDAssigner::IdentifierType> cmpBbSet;
  for (auto &F : M) {
    IdMap[&F] = IdentifierGenerator->getUniqueIdentifier();
    for (Value &Arg : F.args()) {
      IdMap[&Arg] = IdentifierGenerator->getUniqueIdentifier();
    }

    for (auto &BB : F) {
      IdMap[&BB] = IdentifierGenerator->getUniqueIdentifier();
      for (auto &I : BB) {
        IdMap[&I] = IdentifierGenerator->getUniqueIdentifier();
      }
    }

    for (auto &BB : F) {
      const auto *TInst = BB.getTerminator();
      auto srcId = IdMap[&BB];
      for (unsigned I = 0, NSucc = TInst->getNumSuccessors(); I < NSucc; ++I) {
          BasicBlock *Succ = TInst->getSuccessor(I);
          auto dstId = IdMap[&*Succ];
          CfgEdges.insert({srcId, dstId});
      }
      cmpBbSet.insert(srcId);
      for (auto &I : BB) {
          if (CallInst *callInst = dyn_cast<CallInst>(&I)) {
              if (Function *calledFunction = callInst->getCalledFunction()) {
                  if (TraceFunctions.count(calledFunction->getName()) == 0)
                          continue;

                  auto argIndex = 1;
                  // Angora track mode functions have cmpid as arg 0
                  // fast mode have it as arg 1
                  if (calledFunction->getName().endswith("_tt"))
                      argIndex = 0;
                  // Exception to the rule
                  if (calledFunction->getName() == "__angora_trace_switch")
                      argIndex = 0;
                  if (callInst->getNumArgOperands() < (unsigned) (argIndex == 0 ? 1 : argIndex - 1))
                      continue;
                  auto CmpIdArg = callInst->getArgOperand(argIndex);
                  int32_t cmpId = 0;
                  if (ConstantInt* CI = dyn_cast<ConstantInt>(CmpIdArg)) {
                      cmpId = CI->getSExtValue();

                      // Store BB to Angora CMP mapping
                      for (auto bb_id: cmpBbSet) {
                        IdToAngoraMap[bb_id] = cmpId;
                      }

                      // Store Angora CMP to BB id mapping
                      CmpMap[cmpId] = cmpBbSet;
                      cmpBbSet = std::set<IDAssigner::IdentifierType>();
                  }
              }
          }
      }
    }

    collectCallSiteDominators(&F);
  }

  if (!ClCustomTargetsFile.empty())
      addCustomTargetsFromFile(ClCustomTargetsFile, &M);

  if (ClEmitInfo) {
    emitInfoFile(ClInfoFile);
  }
  if (ClEmitCfg) {
    emitCfgFile(ClCfgFile);
    emitCmpMapFile("cmp.map");
  }

  return false;
}


// Get Cmps that correspond to the specified instruction
void IDAssigner::collectPreviousIndirectBranch(Instruction *Inst, SmallPtrSet<Instruction *, 16> *Result, SmallPtrSet<Instruction *, 16> *Seen) {
    if (Seen->count(Inst) > 0)
        return;

    Seen->insert(Inst);

    if (isa<CmpInst>(Inst)) {
        CmpInst *CI = dyn_cast<CmpInst>(Inst);
        if (CI) {
            Result->insert(CI);
        }
        return;
    } else if (isa<BranchInst>(Inst)) {
        BranchInst *BI = dyn_cast<BranchInst>(Inst);
        if (BI->isConditional()) {
           Value *V = BI->getCondition();
           if (isa<Instruction>(V)) {
                collectPreviousIndirectBranch(dyn_cast<Instruction>(V), Result, Seen);
           }
        }

        // ParmeSan: recursively get all indirect call dominators
        if (ClFollowIndDominators) {
            for (BasicBlock *Pred: predecessors(BI->getParent())) {
               Instruction *Term = Pred->getTerminator();
               collectPreviousIndirectBranch(Term, Result, Seen);
            }
        }
        return;
    } else if (LoadInst *LI = dyn_cast<LoadInst>(Inst)) {
        // Get function pointer
        Value *V = LI->getPointerOperand();
        if (V) {
            for (auto U: V->users()) {
                // Collect branch for all users
                if (Instruction *I = dyn_cast<Instruction>(U)) {
                   collectPreviousIndirectBranch(I, Result, Seen);
                }
            }
        }
        return;
    } else if (StoreInst *SI = dyn_cast<StoreInst>(Inst)) {
        //errs() << "Found store to fptr: " << *SI << "\n";
        for (BasicBlock *Pred: predecessors(SI->getParent())) {
           Instruction *Term = Pred->getTerminator();
           collectPreviousIndirectBranch(Term, Result, Seen);
        }
    }
    
}

void IDAssigner::getAnalysisUsage(AnalysisUsage &Info) const {
  Info.setPreservesAll();
}

static void writeFunctionName(raw_ostream &O, const Value *Val) {
  if (const auto *F = dyn_cast<Function>(Val)) {
    O << F->getName();
  } else if (const auto *Arg = dyn_cast<Argument>(Val)) {
    if (const auto F = Arg->getParent()) {
      O << F->getName();
    }
  } else if (const auto *BB = dyn_cast<BasicBlock>(Val)) {
    if (const auto F = BB->getParent()) {
      O << F->getName();
    }
  } else if (const auto *Inst = dyn_cast<Instruction>(Val)) {
    if (const auto *F = Inst->getFunction()) {
      O << F->getName();
    }
  } else {
    llvm_unreachable("Unknown Value encountered");
  }
}

static void writeInstDebugInfo(const Instruction *I, raw_ostream &O) {
  auto Loc = I->getDebugLoc();
  if (!Loc)
    return;
  auto Dir = Loc->getDirectory();
  if (!Dir.empty())
    O << Dir << "/";
  Loc.print(O);
}

static void writeDebugInfo(raw_ostream &O, const Value *Val) {
  if (const auto *F = dyn_cast<Function>(Val)) {
    if (!F->empty()) {
      if (auto *FirstInst =
              F->getEntryBlock().getFirstNonPHIOrDbgOrLifetime()) {
        writeInstDebugInfo(FirstInst, O);
      }
    }
  } else if (const auto *Arg = dyn_cast<Argument>(Val)) {
    if (const auto F = Arg->getParent()) {
      if (!F->empty()) {
        if (auto *FirstInst =
                F->getEntryBlock().getFirstNonPHIOrDbgOrLifetime()) {
          writeInstDebugInfo(FirstInst, O);
        }
      }
    }
  } else if (const auto *BB = dyn_cast<BasicBlock>(Val)) {
    if (auto *FirstInst = BB->getFirstNonPHIOrDbgOrLifetime()) {
      writeInstDebugInfo(FirstInst, O);
    }
  } else if (const auto *Inst = dyn_cast<Instruction>(Val)) {
    writeInstDebugInfo(Inst, O);
  } else {
    llvm_unreachable("Unknown Value encountered");
  }
}

void IDAssigner::print(raw_ostream &O, const Module *M) const {
  for (const auto &Iter : IdMap) {
    O << formatv("{0,16:x}: ", Iter.second);

    if (isa<Function>(Iter.first)) {
      O << "Function:\t";
    } else if (isa<Argument>(Iter.first)) {
      O << "Argument:\t";
    } else if (isa<BasicBlock>(Iter.first)) {
      O << "BasicBlock:\t";
    } else if (isa<Instruction>(Iter.first)) {
      O << "Instruction:\t";
    } else {
      llvm_unreachable("Unknown Value associated with ID");
    }

    O << "(";
    writeFunctionName(O, Iter.first);
    O << ") ";
    writeDebugInfo(O, Iter.first);
    O << "\n";
  }
}

const IDAssigner::IdentifiersMap &IDAssigner::getIdentifiersMap() const {
  return IdMap;
}

const IDAssigner::CmpsMap &IDAssigner::getCmpMap() const {
  return CmpMap;
}

const IDAssigner::CallSiteDominators &IDAssigner::getCallSiteDominators() const {
  return CallSiteDominatorsMap;
}

const IDAssigner::CmpsCfg IDAssigner::getCmpCfg() const {
  IDAssigner::CmpsCfg result;
  std::map<IdentifierType, CmpIdType> rev_map;
  for (auto e : CmpMap) {
      for (auto bb: e.second) {
          rev_map.insert({bb, e.first});
      }
  }

  for (auto e : CfgEdges) {
    IdentifierType src,dst;
    std::tie(src,dst) = e;
    result.insert({rev_map[src], rev_map[dst]});
  }
  return result;
}

// Stolen from AFLGo to be (somewhat) compatible with the same targets file
// https://github.com/aflgo/aflgo/blob/master/llvm_mode/afl-llvm-pass.so.cc
void IDAssigner::addCustomTargetsFromFile(const std::string Path, Module *M) {
    
    if (Path.empty())
        return;

    std::list<std::string> targets;
    std::ifstream targetsfile(Path);
    if (targetsfile.fail()) {
        errs() << "Could not open targets file: " << Path << "\n";
        return;
    }
    std::string line;

    while (std::getline(targetsfile, line)) {
      targets.push_back(line);
    }


    LLVMContext &C = M->getContext();

    VoidTy = Type::getVoidTy(C);
    Int32Ty = IntegerType::getInt32Ty(C);
    Type *ParmeSanDummyCallArgs[1] = {Int32Ty};
    ParmeSanDummyCallTy = FunctionType::get(VoidTy, ParmeSanDummyCallArgs, false);
    ParmeSanDummyCall = M->getOrInsertFunction("__parmesan_custom_target", ParmeSanDummyCallTy);

    targetsfile.close();
    for (auto &F : *M) {
      bool is_target = false;
      for (auto &BB : F) {

        std::string filename;
        unsigned line;

        for (auto &I : BB) {
            if (is_target)
                break;
            getDebugLoc(&I, filename, line);
            static const std::string Xlibs("/usr/");
            if (filename.empty() || line == 0 || !filename.compare(0, Xlibs.size(), Xlibs))
                continue;
            for (auto &target : targets) {
                std::size_t found = target.find_last_of("/\\");
                if (found != std::string::npos)
                  target = target.substr(found + 1);

                std::size_t pos = target.find_last_of(":");
                std::string target_file = target.substr(0, pos);
                unsigned int target_line = atoi(target.substr(pos + 1).c_str());

                // Is target match
                if (!target_file.compare(filename) && target_line == line) {
                  is_target = true;
                  IRBuilder<> IRB(&I);
                  // Can be used for debugging and tracing
                  // Creates a call to __parmesan_custom_target(instruction_id)
                  // When loking at the diff, this inserted call will be
                  // considered a change, so the previous conditional becomes a
                  // target
                  Value *CustomTargetId = ConstantInt::get(Int32Ty, IdMap[&I]);
                  Value *Args = {CustomTargetId};
                  IRB.CreateCall(ParmeSanDummyCall, Args);
                }
            }
        }
      }
    }
}

// Stolen from AFLGo
// https://github.com/aflgo/aflgo/blob/master/llvm_mode/afl-llvm-pass.so.cc#L128
void IDAssigner::getDebugLoc(const Instruction *I, std::string &Filename,
                        unsigned &Line) {
if (DILocation *Loc = I->getDebugLoc()) {
    Line = Loc->getLine();
    Filename = Loc->getFilename().str();

    if (Filename.empty()) {
      DILocation *oDILoc = Loc->getInlinedAt();
      if (oDILoc) {
        Line = oDILoc->getLine();
        Filename = oDILoc->getFilename().str();
      }
    }
  }
}

void IDAssigner::emitInfoFile(const std::string Path) const {
  std::error_code EC;
  raw_fd_ostream InfoFile(Path, EC);
  if (EC) {
    errs() << formatv("Could not open info file: {0}\n", Path);
    return;
  }

  InfoFile << "id,type,function,debug_info\n";
  for (const auto &Iter : IdMap) {
    InfoFile << formatv("{0:x},", Iter.second);

    if (isa<Function>(Iter.first)) {
      InfoFile << "func,";
    } else if (isa<Argument>(Iter.first)) {
      InfoFile << "farg,";
    } else if (isa<BasicBlock>(Iter.first)) {
      InfoFile << "babl,";
    } else if (isa<Instruction>(Iter.first)) {
      InfoFile << "inst,";
    } else {
      llvm_unreachable("Unknown Value associated with ID");
    }

    writeFunctionName(InfoFile, Iter.first);
    InfoFile << ",\"";
    writeDebugInfo(InfoFile, Iter.first);
    InfoFile << "\"\n";
  }
}

void IDAssigner::emitCfgFile(const std::string Path) const {
  std::error_code EC;
  raw_fd_ostream InfoFile(Path, EC);
  if (EC) {
    errs() << formatv("Could not open info file: {0}\n", Path);
    return;
  }

  InfoFile << "src,dst\n";
  for (auto e : CfgEdges) {
    IdentifierType src,dst;
    std::tie(src,dst) = e;
    InfoFile << formatv("{0},", (uint64_t)src) << formatv("{0}", (uint64_t)dst) << "\n";
  }
}

void IDAssigner::emitCmpMapFile(const std::string Path) const {
  std::error_code EC;
  raw_fd_ostream InfoFile(Path, EC);
  if (EC) {
    errs() << formatv("Could not open info file: {0}\n", Path);
    return;
  }

  InfoFile << "cmpId,bbId\n";
  for (const auto &Iter : CmpMap) {
      for (const auto &E: Iter.second) {
          // Note: ParmeSan expects the id as a u32, not i32
          InfoFile << formatv("{0},", (uint32_t)Iter.first) << formatv("{0}", (uint64_t)E) << "\n";
      }
  }
}
static RegisterPass<IDAssigner> X{
    "idassign", "IDAssigner: assign unique IDs to LLVM IR values.", true, true};
