//===--- LiteralCastCheck.cpp - clang-tidy --------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "LiteralCastCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang::tidy::misc {

static StringRef getRawStringRef(const SourceRange &Range,
                                 const SourceManager &Sources,
                                 const LangOptions &LangOpts) {
  CharSourceRange TextRange = Lexer::getAsCharRange(Range, Sources, LangOpts);
  return Lexer::getSourceText(TextRange, Sources, LangOpts);
}

LiteralCastCheck::LiteralCastCheck(StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context), CharRegex("^(u8|u|U|L)?"),
      IntRegex(
          "(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?)|([uU]?[zZ]?)|([zZ]?[uU]?))?$"),
      FloatRegex(
          "([fF]|[lL]|([fF]16)|([fF]32)|([fF]64)|([fF]128)|((bf|BF)16))?$"),
      CharPrefix({
          {"char", ""},
          {"char8_t", "u8"},
          {"char16_t", "u"},
          {"char32_t", "U"},
          {"wchar_t", "L"},
      }),
      IntSuffix({
          {"int", ""},
          {"unsigned int", "u"},
          {"long", "l"},
          {"unsigned long", "ul"},
          {"long long", "ll"},
          {"unsigned long long", "ull"},
          {"std::size_t", "uz"},
      }),
      FloatSuffix({
          {"double", ""},
          {"float", "f"},
          {"long double", "l"},
          {"std::float16_t", "f16"},
          {"std::float32_t", "f32"},
          {"std::float64_t", "f64"},
          {"std::float128_t", "f128"},
          {"std::bfloat16_t", "bf16"},
          {"float16_t", "f16"},
          {"float32_t", "f32"},
          {"float64_t", "f64"},
          {"float128_t", "f128"},
          {"bfloat16_t", "bf16"},
      }) {

  assert(CharRegex.isValid());
  assert(IntRegex.isValid());
  assert(FloatRegex.isValid());
}

void LiteralCastCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(explicitCastExpr(has(ignoringParenImpCasts(anyOf(
                                          characterLiteral().bind("character"),
                                          integerLiteral().bind("integer"),
                                          floatLiteral().bind("floating")))))
                         .bind("expr"),
                     this);
}

void LiteralCastCheck::check(const MatchFinder::MatchResult &Result) {

  const SourceManager &Sources = *Result.SourceManager;
  const auto *MatchedCast = Result.Nodes.getNodeAs<ExplicitCastExpr>("expr");
  QualType CastType = MatchedCast->getType();

  std::string Fix;

  const CharacterLiteral *CharLit = NULL;
  const IntegerLiteral *IntLit = NULL;
  const FloatingLiteral *FloatLit = NULL;

  if ((CharLit = Result.Nodes.getNodeAs<CharacterLiteral>("character")) &&
      CharPrefix.contains(CastType.getAsString())) {

    StringRef LitText =
        getRawStringRef(CharLit->getLocation(), Sources, getLangOpts());

    Fix.append(CharPrefix[CastType.getAsString()]);
    Fix.append(CharRegex.sub("", LitText.str()));
  } else if ((IntLit = Result.Nodes.getNodeAs<IntegerLiteral>("integer")) &&
             IntSuffix.contains(CastType.getAsString())) {

    StringRef LitText =
        getRawStringRef(IntLit->getLocation(), Sources, getLangOpts());

    Fix.append(IntRegex.sub("", LitText.str()));
    Fix.append(IntSuffix[CastType.getAsString()]);
  } else if ((FloatLit = Result.Nodes.getNodeAs<FloatingLiteral>("floating")) &&
             FloatSuffix.contains(CastType.getAsString())) {

    StringRef LitText =
        getRawStringRef(FloatLit->getLocation(), Sources, getLangOpts());

    Fix.append(FloatRegex.sub("", LitText.str()));
    Fix.append(FloatSuffix[CastType.getAsString()]);
  }

  diag(MatchedCast->getExprLoc(), "prefer literal sequences instead of casts")
      << FixItHint::CreateReplacement(MatchedCast->getSourceRange(),
                                      Fix.c_str());
}

// TODO coding style and formatting

} // namespace clang::tidy::misc
