//===-------------- EvolutionModule.cpp - clang-tidy ----------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "../ClangTidy.h"
#include "../ClangTidyModule.h"
#include "../ClangTidyModuleRegistry.h"
#include "ContainerFirstElementAccessCheck.h"
#include "OverloadingWithVirtualCheck.h"
#include "ProTypeMemberInitCheck.h"
#include "SymbolNonCopyingConstructorCheck.h"
#include "ZeroVectorInitCheck.h"

namespace clang {
namespace tidy {
namespace evolution {

/// A module containing checks of the C++ Core Guidelines
class EvolutionModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<ContainerFirstElementAccessCheck>(
        "evolution-container-first-element-access");
    CheckFactories.registerCheck<OverloadingWithVirtualCheck>(
        "evolution-overloading-with-virtual");
    CheckFactories.registerCheck<ProTypeMemberInitCheck>(
        "evolution-pro-type-member-init");
    CheckFactories.registerCheck<SymbolNonCopyingConstructorCheck>(
        "evolution-symbol-non-copying-constructor");
    CheckFactories.registerCheck<ZeroVectorInitCheck>(
        "evolution-zero-vector-init");
  }

  ClangTidyOptions getModuleOptions() override {
    ClangTidyOptions Options;

    return Options;
  }
};

// Register the LLVMTidyModule using this statically initialized variable.
static ClangTidyModuleRegistry::Add<EvolutionModule>
    X("evolution-module", "Adds checks for Evolution code standards.");

} // namespace evolution

// This anchor is used to force the linker to link in the generated object file
// and thus register the EvolutionModule.
volatile int EvolutionModuleAnchorSource = 0;

} // namespace tidy
} // namespace clang
