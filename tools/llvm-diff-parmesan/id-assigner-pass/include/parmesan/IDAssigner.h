#ifndef PARMESAN_IDASSIGNER_H
#define PARMESAN_IDASSIGNER_H

#include "llvm/IR/Value.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"
#include "llvm/IR/Instruction.h"
#include "llvm/ADT/SmallPtrSet.h"

#include <cstdint>
#include <memory>
#include <string>
#include <map>
#include <set>

namespace parmesan {
class IDAssigner : public llvm::ModulePass {
public:
  using IdentifierType = std::uint64_t;
  using CmpIdType = std::int32_t;
  using CallSiteIdType = std::int32_t;
  using IdentifiersMap = llvm::ValueMap<const llvm::Value *, IdentifierType>;
  using CmpsMap = std::map<CmpIdType, std::set<IdentifierType>>;
  using CmpsCfg = std::set<std::tuple<CmpIdType, CmpIdType>>;
  using CallSiteDominators = std::map<CallSiteIdType, std::set<CmpIdType>>;
  using IdAngoraMap = std::map<IdentifierType, CmpIdType>;

  static char ID;
  IDAssigner();
  ~IDAssigner();

  bool runOnModule(llvm::Module &M) override;
  void getAnalysisUsage(llvm::AnalysisUsage &Info) const override;
  void print(llvm::raw_ostream &O, const llvm::Module *M) const override;

  const IdentifiersMap &getIdentifiersMap() const;
  const CmpsMap &getCmpMap() const;
  const CmpsCfg getCmpCfg() const;
  const CallSiteDominators &getCallSiteDominators() const;

private:
  class IDGenerator;
  std::unique_ptr<IDGenerator> IdentifierGenerator;

  IdentifiersMap IdMap;
  CmpsMap CmpMap;
  CallSiteDominators CallSiteDominatorsMap;
  IdAngoraMap IdToAngoraMap;

  void collectCallSiteDominators(llvm::Function *F);
  void collectPreviousIndirectBranch(llvm::Instruction *Inst, llvm::SmallPtrSet<llvm::Instruction *, 16> *Result, llvm::SmallPtrSet<llvm::Instruction *, 16> *Seen);

  CmpIdType getAngoraCmpIdForBB(llvm::BasicBlock *BB);


  void emitInfoFile(const std::string Path) const;
  void emitCfgFile(const std::string Path) const;
  void emitCmpMapFile(const std::string Path) const;
  void addCustomTargetsFromFile(const std::string Path, llvm::Module *M);
  void getDebugLoc(const llvm::Instruction *I, std::string &Filename, unsigned &Line);
};
} // namespace parmesan

#endif
