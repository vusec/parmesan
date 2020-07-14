#include "collabfuzz/IDAssigner.h"

#include "llvm/ADT/Statistic.h"
#include "llvm/IR/Argument.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/FormatVariadic.h"
#include "llvm/Support/raw_ostream.h"

#include<set>

#include <fstream>
#include <limits>
#include <sstream>

#define DEBUG_TYPE "idassign"

using namespace collabfuzz;
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
//std::map<unsigned long, IDAssigner::IdentifierType> CmpMap;
//std::map<int32_t, std::set<IDAssigner::IdentifierType>> CmpMap;

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
                  if (calledFunction->getName() != "__angora_trace_cmp" && calledFunction->getName() != "__dfsw___angora_trace_cmp_tt")
                      continue;


                  auto CmpIdArg = callInst->getArgOperand(1);
                  int32_t cmpId = 0;
                  if (ConstantInt* CI = dyn_cast<ConstantInt>(CmpIdArg)) {
                      cmpId = CI->getSExtValue();
                      CmpMap[cmpId] = cmpBbSet;
                      cmpBbSet = std::set<IDAssigner::IdentifierType>();
                  }
              }
          }
      }
    }
  }

  if (ClEmitInfo) {
    emitInfoFile(ClInfoFile);
  }
  if (ClEmitCfg) {
    emitCfgFile(ClCfgFile);
    emitCmpMapFile("cmp.map");
  }

  return false;
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
    InfoFile << formatv("{0:x},", src) << formatv("{0:x}", dst) << "\n";
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
      //InfoFile << formatv("{0:x},", Iter.first) << formatv("{0:x}", Iter.second) << "\n";
      for (const auto &E: Iter.second) {
          //InfoFile << formatv("{0:d},", Iter.first) << formatv("{0:x}", E) << "\n";
          // Note: ParmeSan expects the id as a u32, not i32
          InfoFile << formatv("{0},", (uint32_t)Iter.first) << formatv("{0:x}", E) << "\n";
      }
  }
}
static RegisterPass<IDAssigner> X{
    "idassign", "IDAssigner: assign unique IDs to LLVM IR values.", true, true};
