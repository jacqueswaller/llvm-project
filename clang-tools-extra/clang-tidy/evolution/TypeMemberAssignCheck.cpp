//===--- TypeMemberAssignCheck.cpp - clang-tidy ------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "TypeMemberAssignCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "../utils/Matchers.h"
#include "../utils/TypeTraits.h"
#include "llvm/ADT/SmallPtrSet.h"

using namespace clang::ast_matchers;
using namespace clang::tidy::matchers;
using llvm::SmallPtrSet;
using llvm::SmallPtrSetImpl;

namespace clang {
namespace tidy {
namespace evolution {

namespace {
  // Iterate over all the fields in a record type, both direct and indirect (e.g.
  // if the record contains an anonmyous struct).
  template <typename T, typename Func>
  void forEachField(const RecordDecl &Record, const T &Fields, Func &&Fn) {
    for (const FieldDecl *F : Fields) {
      if (F->isAnonymousStructOrUnion()) {
        if (const CXXRecordDecl *R = F->getType()->getAsCXXRecordDecl())
          forEachField(*R, R->fields(), Fn);
      } else {
        Fn(F);
      }
    }
  }

  bool isIncompleteOrZeroLengthArrayType(ASTContext &Context,
                                                QualType T) {
    if (T->isIncompleteArrayType())
      return true;

    while (const ConstantArrayType *ArrayT =
               Context.getAsConstantArrayType(T)) {
      if (!ArrayT->getSize())
        return true;

      T = ArrayT->getElementType();
    }

    return false;
  }

  template<size_t N>
  void removeFieldsInitializedInBody(
      const Stmt &Stmt, ASTContext &Context,
      SmallPtrSet<const FieldDecl *, N> &FieldDecls) {

    const auto BinaryAssignOpMatch = binaryOperator(
        hasOperatorName("="),
        hasLHS(memberExpr(member(fieldDecl().bind("fieldDecl")))));

    /*const auto CXXAssignOpMatch = cxxOperatorCallExpr(
        ast_matchers::isAssignmentOperator(),
        hasArgument(0, memberExpr(member(fieldDecl().bind("fieldDecl")))));*/

    const auto CXXAssignOpMatch =
        cxxOperatorCallExpr(ast_matchers::isAssignmentOperator()); //,
        //hasArgument(0, memberExpr(member(fieldDecl().bind("fieldDecl")))));

    {
      auto Matches =
        match(findAll(BinaryAssignOpMatch), Stmt, Context);

      for (const auto &Match : Matches)
        FieldDecls.erase(Match.getNodeAs<FieldDecl>("fieldDecl"));
    }

    {
      auto Matches = 
        match(findAll(CXXAssignOpMatch), Stmt, Context);

      for (const auto &Match : Matches)
      {
        const FieldDecl *F = Match.getNodeAs<FieldDecl>("fieldDecl");
        FieldDecls.erase(F);
      }
    }

    if (!FieldDecls.empty()) {
      auto Matches =
          match(findAll(cxxMemberCallExpr(callExpr().bind("callExpr"))), Stmt,
                Context);

      for (const auto &Match : Matches) {
        const CallExpr *Expr = Match.getNodeAs<CallExpr>("callExpr");
        const clang::Stmt *StmtBody = Expr->getCalleeDecl()->getBody();
        if (!StmtBody)
          continue;

        {
          auto Matches = match(
            findAll(BinaryAssignOpMatch), *StmtBody, Context);

          for (const auto &Match : Matches)
            FieldDecls.erase(Match.getNodeAs<FieldDecl>("fieldDecl"));
        }

        {
          auto Matches =
              match(findAll(CXXAssignOpMatch), *StmtBody, Context);

          for (const auto &Match : Matches)
            FieldDecls.erase(Match.getNodeAs<FieldDecl>("fieldDecl"));
        }
      }
    }
  }

  StringRef getName(const FieldDecl *Field) { return Field->getName(); }

  template <typename R, typename T>
  std::string
  toCommaSeparatedString(const R &OrderedDecls,
                         const SmallPtrSetImpl<const T *> &DeclsToInit) {
    SmallVector<StringRef, 16> Names;
    for (const T *Decl : OrderedDecls) {
      if (DeclsToInit.count(Decl))
        Names.emplace_back(getName(Decl));
    }
    return llvm::join(Names.begin(), Names.end(), ", ");
  }
}

TypeMemberAssignCheck::TypeMemberAssignCheck(StringRef Name,
                                               ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context),
      IgnoreArrays(Options.get("IgnoreArrays", true)) {}

void TypeMemberAssignCheck::registerMatchers(MatchFinder *Finder) {
  if (!getLangOpts().CPlusPlus)
    return;

  Finder->addMatcher(
      cxxRecordDecl(hasName("Mutex"), hasMethod(cxxMethodDecl(hasOverloadedOperatorName("="),
                                            isPublic(), isUserProvided()).bind("assign_op")))
          .bind("class"),
      this);
}

void TypeMemberAssignCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *ClassDecl =
      Result.Nodes.getNodeAs<CXXRecordDecl>("class");
  const auto *AssignOp =
      Result.Nodes.getNodeAs<CXXMethodDecl>("assign_op");

  if (ClassDecl && AssignOp) {
    if (!AssignOp->getBody())
      return;

    SmallPtrSet<const FieldDecl *, 16> FieldsToInit;
    forEachField(*ClassDecl, ClassDecl->fields(), [&](const FieldDecl *F) {
      if (F->getType()->isArrayType() && IgnoreArrays)
        return;

      if (isIncompleteOrZeroLengthArrayType(*Result.Context, F->getType()))
        return;

      FieldsToInit.insert(F);
    });
    if (FieldsToInit.empty())
      return;

    removeFieldsInitializedInBody(*AssignOp->getBody(), *Result.Context,
                                  FieldsToInit);

    // Collect all fields in order, both direct fields and indirect fields from
    // anonmyous record types.
    SmallVector<const FieldDecl *, 16> OrderedFields;
    forEachField(*ClassDecl, ClassDecl->fields(),
                 [&](const FieldDecl *F) { OrderedFields.push_back(F); });

    // Collect all the fields we need to initialize, including indirect fields.
    SmallPtrSet<const FieldDecl *, 16> AllFieldsToInit;
    forEachField(*ClassDecl, FieldsToInit,
                 [&](const FieldDecl *F) { AllFieldsToInit.insert(F); });

    if (AllFieldsToInit.empty())
      return;

    DiagnosticBuilder Diag =
        diag(AssignOp ? AssignOp->getBeginLoc() : ClassDecl->getLocation(),
                 "assign operator does not initialize these fields: %0")
        << toCommaSeparatedString(OrderedFields, AllFieldsToInit);
  }
}

void TypeMemberAssignCheck::storeOptions(ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "IgnoreArrays", IgnoreArrays);
}

} // namespace evolution
} // namespace tidy
} // namespace clang
