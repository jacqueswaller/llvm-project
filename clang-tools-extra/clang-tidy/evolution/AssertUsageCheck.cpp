//===--- DebugAssertCheck.cpp - clang-tidy --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "AssertUsageCheck.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Lex/PPCallbacks.h"
#include "clang/Lex/MacroArgs.h"
#include "clang/Basic/TokenKinds.h"
#include "llvm/ADT/STLExtras.h"
#include "../utils/OptionsUtils.h"

namespace clang {
namespace tidy {
namespace evolution {

namespace {

class AssertUsageCallbacks : public PPCallbacks {
public:
  AssertUsageCallbacks(AssertUsageCheck *Check,
                       const SourceManager &SM)
      : Check(Check), SM(SM) {}

  void MacroExpands(const Token &MacroNameTok, const MacroDefinition &,
                    SourceRange, const MacroArgs *Args) override {

    StringRef MacroName = MacroNameTok.getIdentifierInfo()->getName();

    if (Check->needCheck(MacroName)) {
      const Token *Arg = Args->getUnexpArgument(0);
      const auto kind = Arg->getKind();
      if (Arg && Args->getArgLength(Arg) == 1 && tok::isStringLiteral(kind)) {
        Check->report(MacroNameTok);
      }
    }
  }
  
private:
  AssertUsageCheck *Check;
  const SourceManager &SM;
};

} // namespace

AssertUsageCheck::AssertUsageCheck(StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context),
      MacroNames(
          utils::options::parseStringList(Options.get("MacroNames", "Assert;DebugAssert;DebugVerify"))) {}

void AssertUsageCheck::storeOptions(
    ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "MacroNames",
                utils::options::serializeStringList(MacroNames));
}

void AssertUsageCheck::registerPPCallbacks(const SourceManager &SM,
                                           Preprocessor *PP,
                                           Preprocessor *) {
  if (getLangOpts().CPlusPlus11 || getLangOpts().C99) {
    PP->addPPCallbacks(llvm::make_unique<AssertUsageCallbacks>(this, SM));
  }
}

void AssertUsageCheck::report(const Token &MacroNameTok) {
  StringRef Message = "macro '%0' uses string literal as expression argument, it will always evaluate to true.";
  diag(MacroNameTok.getLocation(), Message)
      << MacroNameTok.getIdentifierInfo()->getName();
}

bool AssertUsageCheck::needCheck(const StringRef &MacroName) const {
  return std::find_if(std::begin(MacroNames), std::end(MacroNames), [&MacroName](const std::string& string) {
    return MacroName.equals(StringRef(string)); }) != std::end(MacroNames);
}

} // namespace evolution
} // namespace tidy
} // namespace clang