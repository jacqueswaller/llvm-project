//===--- ZeroVectorInitCheck.cpp - clang-tidy -----------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "ZeroVectorInitCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "../utils/Matchers.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace evolution {

void ZeroVectorInitCheck::registerMatchers(MatchFinder *Finder) {
// Only register the matchers for C++; the functionality currently does not
    // provide any benefit to other languages, despite being benign.
    if (!getLangOpts().CPlusPlus)
        return;

    const auto IsVector = hasType(namedDecl(hasName("Vector")));
    const auto IsZeroVector = hasName("ZERO_VECTOR");
    const auto IsVectorCtor = cxxConstructExpr(
          hasDeclaration(cxxMethodDecl(hasName("Vector"))),
          anyOf(
              allOf(
                  argumentCountIs(1),
                  hasArgument(0, floatLiteral(equals(0.f)))
              ),
              allOf(
                  argumentCountIs(3),
                  hasArgument(0, floatLiteral(equals(0.f))),
                  hasArgument(1, floatLiteral(equals(0.f))),
                  hasArgument(2, floatLiteral(equals(0.f)))
              ),
              allOf(
                  argumentCountIs(4),
                  hasArgument(0, floatLiteral(equals(0.f))),
                  hasArgument(1, floatLiteral(equals(0.f))),
                  hasArgument(2, floatLiteral(equals(0.f))),
                  hasArgument(3, floatLiteral(equals(0.f)))
              )
          ));

    Finder->addMatcher(cxxConstructExpr(
          hasDeclaration(cxxConstructorDecl(hasName("Vector"))),
          hasArgument(0, ignoringImpCasts(declRefExpr(IsVector).bind("ZERO_VECTOR-ctor"))),
          unless(hasAncestor(cxxCtorInitializer()))
    ), this);

    Finder->addMatcher(IsVectorCtor.bind("zero-float-ctor"), this);

    Finder->addMatcher(cxxOperatorCallExpr(isAssignmentOperator(), hasArgument(0, IsVector), hasArgument(1, ignoringImpCasts(declRefExpr(IsVector).bind("ZERO_VECTOR-assignment")))),
                       this);
}

void ZeroVectorInitCheck::check(const MatchFinder::MatchResult &Result) {
    const DeclRefExpr* zeroVectorCtor = Result.Nodes.getNodeAs<DeclRefExpr>("ZERO_VECTOR-ctor");
    if(zeroVectorCtor) {
        static const std::string sZeroVector("ZERO_VECTOR");
        std::string nameAsString = zeroVectorCtor->getDecl()->getNameAsString();
        if(nameAsString == sZeroVector) {
            static const StringRef extraParam("Vector()");

            SourceLocation Loc = zeroVectorCtor->getBeginLoc();

            const CXXConstructExpr* ctorExpr = nullptr;

            auto parents = Result.Context->getParents(*zeroVectorCtor);
            if(parents.size() == 1) {
                ctorExpr = parents[0].get<CXXConstructExpr>();
            }

            if(ctorExpr) {
                const Decl* declParent = nullptr;
                const Stmt* stmtParent = nullptr;
                const Type* typeParent = nullptr;

                const VarDecl* varDecl = nullptr;

                parents = Result.Context->getParents(*ctorExpr);

                if(parents.size() == 1) {
                    declParent = parents[0].get<Decl>();
                    stmtParent = parents[0].get<Stmt>();
                    typeParent = parents[0].get<Type>();

                    varDecl = parents[0].get<VarDecl>();
                }

                SourceRange removal;

                if(varDecl) {
                    // Want to remove everything after the name, including the parenthesis
                    std::pair<FileID, unsigned> sourceLocation = Result.SourceManager->getDecomposedLoc(varDecl->getLocation());
                    sourceLocation.second += varDecl->getDeclName().getAsString().size();
                    SourceLocation nameEnd = Result.SourceManager->getComposedLoc(sourceLocation.first, sourceLocation.second);
                    removal = SourceRange(nameEnd, varDecl->getEndLoc());
                } else {
                    // Just remove what's inside of the parenthesis
                    removal = SourceRange(Loc, zeroVectorCtor->getEndLoc());
                }

                if(stmtParent && !dyn_cast<CXXFunctionalCastExpr>(stmtParent)) {
                    diag(Loc, "Use default constructor instead of copying ZERO_VECTOR")
                            << FixItHint::CreateReplacement(removal, extraParam);
                } else {
                    diag(Loc, "Use default constructor instead of copying ZERO_VECTOR")
                            << FixItHint::CreateRemoval(removal);
                }
            }
        }
    }

    const CXXConstructExpr* zeroFloatCtor = Result.Nodes.getNodeAs<CXXConstructExpr>("zero-float-ctor");
    if(zeroFloatCtor) {
        const VarDecl* varDecl = nullptr;
        auto parents = Result.Context->getParents(*zeroFloatCtor);
        if(parents.size() == 1) {
            varDecl = parents[0].get<VarDecl>();
        }

        SourceRange removal;
        if(varDecl) {
            // Want to remove everything after the name, including the parenthesis
            std::pair<FileID, unsigned> sourceLocation = Result.SourceManager->getDecomposedLoc(varDecl->getLocation());
            sourceLocation.second += varDecl->getDeclName().getAsString().size();
            SourceLocation nameEnd = Result.SourceManager->getComposedLoc(sourceLocation.first, sourceLocation.second);
            removal = SourceRange(nameEnd, varDecl->getEndLoc());
        } else {
            // Just remove what's inside of the parenthesis
            auto argIter = zeroFloatCtor->arg_begin();
            SourceLocation startErase = (*argIter)->getBeginLoc();

            SourceLocation endErase;
            while(argIter != zeroFloatCtor->arg_end()) {
                endErase = (*argIter)->getEndLoc();
                ++argIter;
            }
            removal = SourceRange(startErase, endErase);
        }


        diag(removal.getBegin(), "Use default constructor instead of initializing with 0.f arguments")
                << FixItHint::CreateRemoval(removal);
    }

    const DeclRefExpr* zeroVectorAssignment = Result.Nodes.getNodeAs<DeclRefExpr>("ZERO_VECTOR-assignment");
    if(zeroVectorAssignment) {
        static const std::string sZeroVector("ZERO_VECTOR");
        std::string nameAsString = zeroVectorAssignment->getDecl()->getNameAsString();
        if(nameAsString == sZeroVector) {
            static const StringRef extraParam("Vector()");

            SourceLocation Loc = zeroVectorAssignment->getBeginLoc();

            diag(Loc, "Use default constructor instead of copying ZERO_VECTOR")
                << FixItHint::CreateReplacement(SourceRange(Loc, zeroVectorAssignment->getEndLoc()), extraParam);
        }
    }
}

} // namespace evolution
} // namespace tidy
} // namespace clang
