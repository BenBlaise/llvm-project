//===--- LiteralCastCheck.h - clang-tidy ------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MISC_LITERALCASTCHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MISC_LITERALCASTCHECK_H

#include "../ClangTidyCheck.h"
#include "clang/Lex/Lexer.h"
#include "llvm/ADT/StringMap.h"

#include <iostream>

namespace clang::tidy::misc {

/// FIXME: Write a short description.
///
/// For the user-facing documentation see:
/// http://clang.llvm.org/extra/clang-tidy/checks/misc/literal-cast.html
class LiteralCastCheck : public ClangTidyCheck {
public:
  LiteralCastCheck(StringRef Name, ClangTidyContext *Context);
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
private:
  llvm::Regex CharRegex;
  llvm::Regex IntRegex;
  llvm::Regex FloatRegex;
  llvm::StringMap<std::string> CharPrefix;
  llvm::StringMap<std::string> IntSuffix;
  llvm::StringMap<std::string> FloatSuffix;
  //llvm::StringMap<std::string> Modifiers;
};

} // namespace clang::tidy::misc

#endif // LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_MISC_LITERALCASTCHECK_H
