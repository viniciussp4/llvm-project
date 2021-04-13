
#include "llvm/Transforms/IPO/SizeInline.h"

#include "llvm/Transforms/IPO.h"

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

  for (Function *F : AllFunctions) {
    errs() << F->getName() << ": " << F->getInstructionCount() << "\n";
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
