//===--- TypeMemberAssignCheck.h - clang-tidy ----------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_EVOLUTION_PROTYPEMEMBERASSIGNCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_EVOLUTION_PROTYPEMEMBERASSIGNCHECK_H

#include "../ClangTidyCheck.h"

namespace clang {
namespace tidy {
namespace evolution {

/// \brief 
class TypeMemberAssignCheck : public ClangTidyCheck {
public:
  TypeMemberAssignCheck(StringRef Name, ClangTidyContext *Context);
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
  void storeOptions(ClangTidyOptions::OptionMap &Opts) override;

private:
  // Whether arrays need to be initialized or not. Default is false.
  bool IgnoreArrays;
};

} // namespace evolution
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_EVOLUTION_PROTYPEMEMBERASSIGNCHECK_H
