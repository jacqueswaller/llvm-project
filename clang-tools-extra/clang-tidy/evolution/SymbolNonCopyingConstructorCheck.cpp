//===--- SymbolNonCopyingConstructorCheck.cpp - clang-tidy ----------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "SymbolNonCopyingConstructorCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace evolution {

void SymbolNonCopyingConstructorCheck::registerMatchers(MatchFinder *Finder) {
    // Only register the matchers for C++; the functionality currently does not
    // provide any benefit to other languages, despite being benign.
    if (!getLangOpts().CPlusPlus)
        return;

    Finder->addMatcher(
      cxxConstructExpr(
          hasDeclaration(cxxMethodDecl(hasName("Symbol"))),
          hasArgument(0, stringLiteral().bind("literal-arg")),
          argumentCountIs(1))
          .bind("constructor"),
      this);
}

void SymbolNonCopyingConstructorCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *E = Result.Nodes.getNodeAs<CXXConstructExpr>("constructor");
  assert(E && "missing constructor expression");

  SourceLocation Loc = E->getBeginLoc();

  const auto *Literal = Result.Nodes.getNodeAs<StringLiteral>("literal-arg");
  assert(Literal && "missing string literal");

  if(E && Literal) {
    StringRef extraParam(", Symbol::NONCOPYING_CONSTRUCTOR");
  
    diag(E->getBeginLoc(), "Symbol constructed with string literal should use non-copying constructor")
          << FixItHint::CreateInsertion(E->getEndLoc(), extraParam);
  }
}

} // namespace evolution
} // namespace tidy
} // namespace clang
