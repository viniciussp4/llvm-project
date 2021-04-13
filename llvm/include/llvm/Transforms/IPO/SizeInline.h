

#ifndef LLVM_TRANSFORMS_IPO_SIZEINLINE_H
#define LLVM_TRANSFORMS_IPO_SIZEINLINE_H

#include "llvm/InitializePasses.h"
#include "llvm/Pass.h"

#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/Analysis/ProfileSummaryInfo.h"
#include "llvm/Analysis/BlockFrequencyInfo.h"

#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSet.h"

#include "llvm/Transforms/Utils/Cloning.h"

#include <map>
#include <vector>

namespace llvm{

class SizeInlining : public ModulePass {
public:
  static char ID;
  SizeInlining() : ModulePass(ID) {
     initializeSizeInliningPass(*PassRegistry::getPassRegistry());
  }
  bool runOnModule(Module &M) override;
  void getAnalysisUsage(AnalysisUsage &AU) const override;
};

} // namespace
#endif

