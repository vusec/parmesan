//===-- llvm-diff.cpp - Module comparator command-line driver ---*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines the command-line driver for the difference engine.
//
//===----------------------------------------------------------------------===//

#include "DiffLog.h"
#include "DifferenceEngine.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include <string>
#include <utility>
#include "parmesan/IDAssigner.h"


using namespace llvm;
using namespace parmesan;

/// Reads a module from a file.  On error, messages are written to stderr
/// and null is returned.
static std::unique_ptr<Module> readModule(LLVMContext &Context,
                                          StringRef Name) {
  SMDiagnostic Diag;

  std::unique_ptr<Module> M = parseIRFile(Name, Diag, Context);
  if (!M)
    Diag.print("llvm-diff", errs());

 
  /*
  legacy::PassManager passmanager;
  parmesan::IDAssigner *IA = new parmesan::IDAssigner();
  passmanager.add(IA);
  passmanager.run(*M);
  const IDAssigner::IdentifiersMap *IdMap = &IA->getIdentifiersMap();
  for (const auto &Iter : *IdMap) {
      errs() << Iter.first << " " << Iter.second << "n";
  }
  */


  return M;
}

static void diffGlobal(DifferenceEngine &Engine, Module &L, Module &R,
                       StringRef Name) {
  // Drop leading sigils from the global name.
  if (Name.startswith("@")) Name = Name.substr(1);

  Function *LFn = L.getFunction(Name);
  Function *RFn = R.getFunction(Name);
  if (LFn && RFn)
    Engine.diff(LFn, RFn);
  else if (!LFn && !RFn)
    errs() << "No function named @" << Name << " in either module\n";
  else if (!LFn)
    errs() << "No function named @" << Name << " in left module\n";
  else
    errs() << "No function named @" << Name << " in right module\n";
}

static cl::opt<std::string> LeftFilename(cl::Positional,
                                         cl::desc("<first file>"),
                                         cl::Required);
static cl::opt<std::string> RightFilename(cl::Positional,
                                          cl::desc("<second file>"),
                                          cl::Required);
static cl::list<std::string> GlobalsToCompare(cl::Positional,
                                              cl::desc("<globals to compare>"));
static cl::opt<bool> EmitJson("json",
                                      cl::desc("Emit ParmeSan JSON targets (targets.json)"),
                                      cl::init(false), cl::Hidden);

static const IDAssigner::IdentifiersMap *IdMap;
static const IDAssigner::CallSiteDominators *CSDominatorMap;
parmesan::IDAssigner *IA = new parmesan::IDAssigner();
legacy::PassManager passmanager;

void collectIds(Module *M) {
  passmanager.add(IA);
  passmanager.run(*M);
  IdMap = &IA->getIdentifiersMap();
  CSDominatorMap = &IA->getCallSiteDominators();

}

int main(int argc, char **argv) {
  cl::ParseCommandLineOptions(argc, argv);

  LLVMContext Context;

  // Load both modules.  Die if that fails.
  std::unique_ptr<Module> LModule = readModule(Context, LeftFilename);
  std::unique_ptr<Module> RModule = readModule(Context, RightFilename);
  if (!LModule || !RModule) return 1;

  collectIds(LModule.get());

  DiffConsumer Consumer;
  DifferenceEngine Engine(Consumer);

  Consumer.setIdAssigner(IA);

  // If any global names were given, just diff those.
  if (!GlobalsToCompare.empty()) {
    for (unsigned I = 0, E = GlobalsToCompare.size(); I != E; ++I)
      diffGlobal(Engine, *LModule, *RModule, GlobalsToCompare[I]);

  // Otherwise, diff everything in the module.
  } else {
    Engine.diff(LModule.get(), RModule.get());
  }

  Consumer.printStats();
  if (EmitJson) {
      std::error_code EC;
      raw_fd_ostream InfoFile("targets.json", EC);
      Consumer.printStatsJson(InfoFile);
  }
  return Consumer.hadDifferences();
}
