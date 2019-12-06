//===--- DebugAssertCheck.h - clang-tidy ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_EVOLUTION_DEBUGASSERTCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_EVOLUTION_DEBUGASSERTCHECK_H

#include "../ClangTidyCheck.h"

namespace clang {
namespace tidy {
namespace evolution {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/evolution-debug-assert.html
class AssertUsageCheck : public ClangTidyCheck {
public:
  AssertUsageCheck(StringRef Name, ClangTidyContext *Context);

  void storeOptions(ClangTidyOptions::OptionMap &Opts) override;

  void registerPPCallbacks(const SourceManager &SM, Preprocessor *PP,
                           Preprocessor *ModuleExpanderPP) override;

  void report(const Token &MacroNameTok);
  bool needCheck(const StringRef &MacroName) const;

private:
  const std::vector<std::string> MacroNames;
};

} // namespace evolution
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_EVOLUTION_DEBUGASSERTCHECK_H
