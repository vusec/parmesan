#ifndef COLLABFUZZ_IDASSIGNER_H
#define COLLABFUZZ_IDASSIGNER_H

#include "llvm/IR/Value.h"
#include "llvm/IR/ValueMap.h"
#include "llvm/Pass.h"

#include <cstdint>
#include <memory>
#include <string>
#include <map>
#include <set>

namespace collabfuzz {
class IDAssigner : public llvm::ModulePass {
public:
  using IdentifierType = std::uint64_t;
  using CmpIdType = std::int32_t;
  using IdentifiersMap = llvm::ValueMap<const llvm::Value *, IdentifierType>;
  using CmpsMap = std::map<CmpIdType, std::set<IdentifierType>>;

  static char ID;
  IDAssigner();
  ~IDAssigner();

  bool runOnModule(llvm::Module &M) override;
  void getAnalysisUsage(llvm::AnalysisUsage &Info) const override;
  void print(llvm::raw_ostream &O, const llvm::Module *M) const override;

  const IdentifiersMap &getIdentifiersMap() const;
  const CmpsMap &getCmpMap() const;

private:
  class IDGenerator;
  std::unique_ptr<IDGenerator> IdentifierGenerator;

  IdentifiersMap IdMap;
  CmpsMap CmpMap;

  void emitInfoFile(const std::string Path) const;
  void emitCfgFile(const std::string Path) const;
  void emitCmpMapFile(const std::string Path) const;
};
} // namespace collabfuzz

#endif
