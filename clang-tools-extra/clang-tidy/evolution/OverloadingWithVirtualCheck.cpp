//===--- OverloadingWithVirtualCheck.cpp - clang-tidy ---------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "OverloadingWithVirtualCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace evolution {

void OverloadingWithVirtualCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      cxxMethodDecl(isVirtual(), unless(anyOf(isDefaulted(), isOverride())),
                    hasParent(cxxRecordDecl(isDerivedFrom(anything()))))
          .bind("virtualMethod"),
      this);
}

void OverloadingWithVirtualCheck::check(
    const MatchFinder::MatchResult &Result) {

  const auto *MatchedDecl =
      Result.Nodes.getNodeAs<CXXMethodDecl>("virtualMethod");
  if (!MatchedDecl) {
    return;
  }

  const auto *ParentDecl = MatchedDecl->getParent();
  if (!ParentDecl) {
    return;
  }

  DeclarationName MatchName = MatchedDecl->getDeclName();
  if (MatchName.getNameKind() != DeclarationName::Identifier) {
    return;
  }

  ParentDecl->forallBases([&MatchedDecl, &MatchName,
                           this](const CXXRecordDecl *Base) {

    size_t ParamSize = MatchedDecl->param_size();
    auto Results = Base->lookup(MatchName);
    for (const auto& Result: Results) {
      if (const CXXMethodDecl *Method = dyn_cast<CXXMethodDecl>(Result)) {
        if (Method->isVirtual() || Method->param_size() != ParamSize) {
          continue;
        }

        bool Match = Method->getReturnType() == MatchedDecl->getReturnType();
        if (ParamSize != 0) {
          auto MethodParam = Method->param_begin();
          auto MatchParam = MatchedDecl->param_begin();

          for (size_t i = 0; i < ParamSize; ++i) {
            Match &= (*MethodParam)->getType() == (*MatchParam)->getType();
          }
        }

        if (Match) {
          FixItHint Hint;
          diag(MatchedDecl->getBeginLoc(),
               (llvm::Twine("method overloads non virtual method from base ") +
                Base->getName() + llvm::Twine(" class."))
                   .str())
              << Hint;

          diag(Method->getBeginLoc(), "overloaded method.", DiagnosticIDs::Note);
        }
      }
    }

    return true;
  });
}

} // namespace evolution
} // namespace tidy
} // namespace clang
