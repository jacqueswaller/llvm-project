//===--- ProTypeMemberInitCheck.h - clang-tidy-------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_EVOLUTION_PRO_TYPE_MEMBER_INIT_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_EVOLUTION_PRO_TYPE_MEMBER_INIT_H

#include "../ClangTidy.h"

namespace clang {
namespace tidy {
namespace evolution {

/// \brief Modified version of cppcoreguidelines-pro-type-member-init
class ProTypeMemberInitCheck : public ClangTidyCheck {
public:
  ProTypeMemberInitCheck(StringRef Name, ClangTidyContext *Context);
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
  void storeOptions(ClangTidyOptions::OptionMap &Opts) override;

private:
  void checkMissingMemberInitializer(ASTContext &Context,
                                     const CXXRecordDecl &ClassDecl,
                                     const CXXConstructorDecl *Ctor);

  void checkMissingBaseClassInitializer(const ASTContext &Context,
                                        const CXXRecordDecl &ClassDecl,
                                        const CXXConstructorDecl *Ctor);

  void checkUninitializedTrivialType(const ASTContext &Context,
                                     const VarDecl *Var);

  // Whether arrays need to be initialized or not. Default is false.
  bool IgnoreArrays;
  bool IgnoreDefaultConstructible;
  bool IgnoreVars;
  
  // Whether fix-its for initialization of fundamental type use assignment
  // instead of brace initalization. Only effective in C++11 mode. Default is
  // false.
  bool UseAssignment;
};

} // namespace evolution
} // namespace tidy
} // namespace clang

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_EVOLUTION_PRO_TYPE_MEMBER_INIT_H
