
#include "llvm/Transforms/IPO/SizeInline.h"

#include "llvm/Support/CommandLine.h"

#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <vector>
#include <algorithm>

using namespace llvm;

static cl::opt<unsigned> TinyThreshold (
    "size-inline-trivial-tiny-threshold", cl::Hidden,
    cl::desc("Always inine functions smaler than this threshold"),
    cl::init(2));

static cl::opt<unsigned> TrivialNumAttempts (
    "size-inline-trivial-attempts", cl::Hidden,
    cl::desc("Iteratively perform the trivial inline"),
    cl::init(2));

static bool TrivialInlining(Module &M) {
  std::vector<Function*> AllFunctions;
  for (Function &F : M) {
    if (F.isDeclaration()) continue;
    AllFunctions.push_back(&F);
  }

  std::sort(AllFunctions.begin(), AllFunctions.end(), [&](const Function *F1, const Function *F2) -> bool {
    return F1->getInstructionCount() < F2->getInstructionCount();
  });

  bool Changed = false;
  //from smallest to largest functions, perform trivial inlining
  for (Function *F : AllFunctions) {
    if (F->getInstructionCount()<=TinyThreshold || F->getNumUses()==1) {
      errs() << "Trivially Inlining: " << F->getName() << "\n";
      for (User *U : F->users()) {
        if (CallBase *CB = dyn_cast<CallBase>(U)) {
          InlineFunctionInfo IFI;
	  auto Result = InlineFunction(*CB, IFI);
	  Changed = Changed || Result.isSuccess();
	}
      }
      //aggressive
      if (F->getNumUses()==0) F->eraseFromParent(); //erase if unused
    }
  }

  return Changed;
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
