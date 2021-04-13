
#include "llvm/Transforms/IPO/SizeInline.h"

#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Utils/Cloning.h"

#include <vector>
#include <algorithm>

using namespace llvm;

static bool TrivialInlining(Module &M) {
  std::vector<Function*> AllFunctions;
  for (Function &F : M) {
    if (F.isDeclaration()) continue;
    AllFunctions.push_back(&F);
  }

  std::sort(AllFunctions.begin(), AllFunctions.end(), [&](const Function *F1, const Function *F2) -> bool {
    return F1->getInstructionCount() < F2->getInstructionCount();
  });

  //from smallest to largest functions, perform trivial inlining
  for (Function *F : AllFunctions) {
    if (F->getInstructionCount()<=2 || F->getNumUses()==1) {
      errs() << "Trivially Inlining: " << F->getName() << "\n";
      for (User *U : F->users()) {
        if (CallBase *CB = dyn_cast<CallBase>(U)) {
          InlineFunctionInfo IFI;
	  auto Result = InlineFunction(*CB, IFI);
	}
      }
    }
  }

  return false;
}

bool SizeInlining::runOnModule(Module &M) {
  return TrivialInlining(M);
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
