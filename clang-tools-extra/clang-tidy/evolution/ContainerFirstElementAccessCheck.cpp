//===--- FirstElementAccessCheck.cpp - clang-tidy -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ContainerFirstElementAccessCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace evolution {

void ContainerFirstElementAccessCheck::registerMatchers(MatchFinder *Finder) {

  const auto ValidContainer = qualType(hasUnqualifiedDesugaredType(
      recordType(hasDeclaration(cxxRecordDecl(isSameOrDerivedFrom(
          namedDecl(
              has(cxxMethodDecl(hasOverloadedOperatorName("[]"))
                      .bind("operator[]")),
              has(cxxMethodDecl(
                      parameterCountIs(0), isPublic(), hasName("data"),
                      returns(pointerType(pointee(unless(isConstQualified())))))
                      .bind("data")))
              .bind("container")))))));

  Finder->addMatcher(
      cxxOperatorCallExpr(
          hasOverloadedOperatorName("[]"),
          hasArgument(0, expr(anyOf(hasType(ValidContainer),
                                    hasType(pointsTo(ValidContainer)),
                                    hasType(references(ValidContainer))))),
          hasArgument(1, integerLiteral(equals(0))),
          hasParent(unaryOperator(hasOperatorName("&"))))
          .bind("FirstElemInContainer"),
      this);

  const auto ValidConstContainer = qualType(hasUnqualifiedDesugaredType(
      recordType(hasDeclaration(cxxRecordDecl(isSameOrDerivedFrom(
          namedDecl(
              has(cxxMethodDecl(hasOverloadedOperatorName("[]"))
                      .bind("operator[]")),
              has(cxxMethodDecl(
                      parameterCountIs(0), isPublic(), hasName("data"),
                      returns(pointerType(pointee(isConstQualified()))))
                      .bind("const_data")))
              .bind("const_container")))))));

  Finder->addMatcher(
      cxxOperatorCallExpr(
          hasOverloadedOperatorName("[]"),
          hasArgument(0, expr(anyOf(hasType(ValidConstContainer),
                                    hasType(pointsTo(ValidConstContainer)),
                                    hasType(references(ValidConstContainer))))),
          hasArgument(1, integerLiteral(equals(0))),
          hasParent(unaryOperator(
              hasOperatorName("&"),
              hasParent(implicitCastExpr()))))
          .bind("FirstElemInConstContainer"),
      this);

}

void ContainerFirstElementAccessCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *ContainterOperatorCall =
      Result.Nodes.getNodeAs<CXXOperatorCallExpr>("FirstElemInContainer");

  FixItHint Hint;
  if (ContainterOperatorCall) {
    diag(ContainterOperatorCall->getBeginLoc(),
         "the 'data' method should be used to access address of first element "
         "of the container")
        << Hint;
  }

  const auto *ConstContainterOperatorCall =
      Result.Nodes.getNodeAs<CXXOperatorCallExpr>("FirstElemInConstContainer");

  if (ConstContainterOperatorCall) {
    diag(ConstContainterOperatorCall->getBeginLoc(),
         "the 'data' method should be used to access address of first element "
         "of the container")
        << Hint;
  }
}

} // namespace evolution
} // namespace tidy
} // namespace clang
