
#include "llvm/Transforms/IPO/SizeInline.h"

#include "llvm/Analysis/CallGraph.h"

#include "llvm/Support/CommandLine.h"

#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <vector>
#include <algorithm>

using namespace llvm;

static cl::opt<unsigned> TinyThreshold (
    "size-inline-tiny-threshold", cl::Hidden,
    cl::desc("Always inine functions smaler than this threshold"),
    cl::init(2));

static cl::opt<unsigned> SolitaryMaxThreshold (
    "size-inline-solitary-max-threshold", cl::Hidden,
    cl::desc("Always inine functions smaler than this threshold"),
    cl::init(1000));

static cl::opt<unsigned> TrivialNumAttempts (
    "size-inline-trivial-attempts", cl::Hidden,
    cl::desc("Iteratively perform the trivial inline"),
    cl::init(5));

static bool TrivialInlining(Module &M) {
  std::vector<Function*> AllFunctions;
  for (Function &F : M) {
    if (F.isDeclaration()) continue;
    AllFunctions.push_back(&F);
  }

  std::sort(AllFunctions.begin(), AllFunctions.end(), [&](const Function *F1, const Function *F2) -> bool {
    return F1->getInstructionCount() < F2->getInstructionCount();
  });

  unsigned MaxSize = std::max(TinyThreshold, SolitaryMaxThreshold);

  bool Changed = false;
  //from smallest to largest functions, perform trivial inlining
  for (Function *F : AllFunctions) {
    size_t FSize = F->getInstructionCount();
    if (FSize > MaxSize) break; //functions are sorted

    if (FSize<=TinyThreshold || (F->getNumUses()==1 && FSize<=SolitaryMaxThreshold) ) {
      errs() << "Trivially Inlining: " << F->getName() << "\n";
      for (User *U : F->users()) {
        if (CallBase *CB = dyn_cast<CallBase>(U)) {
          InlineFunctionInfo IFI;
	  auto Result = InlineFunction(*CB, IFI);
	  Changed = Changed || Result.isSuccess();
	}
      }
      //aggressive: erase if unused after inlining
      if (F->getNumUses()==0) F->eraseFromParent(); 
    }
  }

  return Changed;
}

static bool SituationalInlining(Module &M) {
  CallGraph CG(M);
  //TODO: create SCC-based graph; navigate over the SCCs of this graph
  //try to perform inlining within these SCCs.
  return true;
}


bool SizeInlining::runOnModule(Module &M) {
  bool Changed = false;
  for (unsigned i = 0; i<TrivialNumAttempts; i++) {
    bool AnyInlining = TrivialInlining(M);
    Changed = Changed || AnyInlining;
    if (!AnyInlining) break;
  }
  return Changed;
}


void SizeInlining::getAnalysisUsage(AnalysisUsage &AU) const {
  ModulePass::getAnalysisUsage(AU);
}

char SizeInlining::ID = 0;
INITIALIZE_PASS(SizeInlining, "size-inline", "New Inlining for Size", false,
                false)

ModulePass *llvm::createSizeInliningPass() {
  return new SizeInlining();
}
