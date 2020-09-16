//===- IndVarSimplify.h - Induction Variable Simplification -----*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file provides the interface for the Induction Variable
// Simplification pass.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_SCALAR_INDVARSIMPLIFY_H
#define LLVM_TRANSFORMS_SCALAR_INDVARSIMPLIFY_H

#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/PassManager.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"
#include "llvm/Analysis/MemorySSA.h"
#include "llvm/Analysis/MemorySSAUpdater.h"
#include "llvm/Analysis/ScalarEvolution.h"
#include "llvm/Analysis/ScalarEvolutionExpressions.h"
#include "llvm/Analysis/TargetLibraryInfo.h"
#include "llvm/Analysis/TargetTransformInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Dominators.h"

#include "llvm/Transforms/Utils/ScalarEvolutionExpander.h"

namespace llvm {

class Loop;
class LPMUpdater;

class IndVarSimplify {
  LoopInfo *LI;
  ScalarEvolution *SE;
  DominatorTree *DT;
  const DataLayout &DL;
  TargetLibraryInfo *TLI;
  const TargetTransformInfo *TTI;
  std::unique_ptr<MemorySSAUpdater> MSSAU;

  SmallVector<WeakTrackingVH, 16> DeadInsts;

  bool handleFloatingPointIV(Loop *L, PHINode *PH);
  bool rewriteNonIntegerIVs(Loop *L);

  bool simplifyAndExtend(Loop *L, SCEVExpander &Rewriter, LoopInfo *LI);
  /// Try to eliminate loop exits based on analyzeable exit counts
  bool optimizeLoopExits(Loop *L, SCEVExpander &Rewriter);
  /// Try to form loop invariant tests for loop exits by changing how many
  /// iterations of the loop run when that is unobservable.
  bool predicateLoopExits(Loop *L, SCEVExpander &Rewriter);

  bool rewriteFirstIterationLoopExitValues(Loop *L);

  bool linearFunctionTestReplace(Loop *L, BasicBlock *ExitingBB,
                                 const SCEV *ExitCount,
                                 PHINode *IndVar, SCEVExpander &Rewriter);

  bool sinkUnusedInvariants(Loop *L);

public:
  IndVarSimplify(LoopInfo *LI, ScalarEvolution *SE, DominatorTree *DT,
                 const DataLayout &DL, TargetLibraryInfo *TLI,
                 TargetTransformInfo *TTI, MemorySSA *MSSA)
      : LI(LI), SE(SE), DT(DT), DL(DL), TLI(TLI), TTI(TTI) {
    if (MSSA)
      MSSAU = std::make_unique<MemorySSAUpdater>(MSSA);
  }

  bool run(Loop *L);
};


class IndVarSimplifyPass : public PassInfoMixin<IndVarSimplifyPass> {
public:
  PreservedAnalyses run(Loop &L, LoopAnalysisManager &AM,
                        LoopStandardAnalysisResults &AR, LPMUpdater &U);
};

} // end namespace llvm

#endif // LLVM_TRANSFORMS_SCALAR_INDVARSIMPLIFY_H
