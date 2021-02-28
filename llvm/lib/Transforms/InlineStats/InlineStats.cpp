#include "llvm/ADT/Statistic.h"
#include "llvm/Analysis/CallGraph.h"
#include "llvm/IR/AbstractCallSite.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Metadata.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/Casting.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

#define DEBUG_TYPE "InlineStats"
STATISTIC(CallsitesNumber, "number_of_initial_callsites");
STATISTIC(FunctionsNumber, "number_of_initial_functions");
STATISTIC(MaxInstsFunction, "max_num_insts_in_a_function");
STATISTIC(MinInstsFunction, "min_num_insts_in_a_function");
STATISTIC(SumInstsFunctions, "sum_num_insts_in_all_function");

namespace {

struct Fprofiler : public ModulePass {

  static char ID;

  Fprofiler() : ModulePass(ID) {}

  virtual bool runOnModule(Module &M) {

    for(auto &F : M) {
      if (F.isDeclaration())
        continue;
      
      uint insts = F.getInstructionCount();
      if(insts > MaxInstsFunction)
        MaxInstsFunction = insts;
      if(insts < MinInstsFunction || MinInstsFunction == 0)
        MinInstsFunction = insts;

      SumInstsFunctions += insts;
      FunctionsNumber++;
    }

    CallGraph CG(M);

    SmallVector<CallBase*, 16> CallSites;

    for (const auto &CGPair : CG) {
      CallGraphNode *Node = CGPair.second.get();
      Function *F = Node->getFunction();
      if (!F || F->isDeclaration())
        continue;

      for (BasicBlock &BB : *F)
        for (Instruction &I : BB) {
          auto *CB = dyn_cast<CallBase>(&I);
          // If this isn't a call, or it is a call to an intrinsic, it can
          // never be inlined.
          if (!CB || isa<IntrinsicInst>(I))
            continue;

          // If this is a direct call to an external function, we can never
          // inline it.  If it is an indirect call, inlining may resolve it to
          // be a direct call, so we keep it.
          if (Function *Callee = CB->getCalledFunction())
            if (Callee->isDeclaration()) { // unavailable definition
              continue;
            }
          
          Function *Caller = CB->getCaller();
          Function *Callee = CB->getCalledFunction();
          if(!Callee)
            continue; 
            
          CallSites.push_back(CB);
        }
    }

    CallsitesNumber = CallSites.size();

    for(auto *CB : CallSites) {
      Function *Caller = CB->getCaller();
      Function *Callee = CB->getCalledFunction();

      if(Caller && Callee) {
        errs() << "\n" << Caller->getName() << " calls " << Callee->getName() << "\n";
      }

    }

    return false;
  } // End RunModule
};

} // namespace
char Fprofiler::ID = 0;
static RegisterPass<Fprofiler> X("inline-stats", "Function Profiler Pass",
                                 false, true);