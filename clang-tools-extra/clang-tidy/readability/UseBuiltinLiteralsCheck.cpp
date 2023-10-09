//===--- UseBuiltinLiteralsCheck.cpp - clang-tidy -------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#include "UseBuiltinLiteralsCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Lex/Lexer.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"

using namespace clang::ast_matchers;

namespace clang::tidy::readability {

namespace {

static StringRef getRawStringRef(const SourceRange &Range,
                                 const SourceManager &Sources,
                                 const LangOptions &LangOpts) {
  CharSourceRange TextRange = Lexer::getAsCharRange(Range, Sources, LangOpts);
  return Lexer::getSourceText(TextRange, Sources, LangOpts);
}

llvm::Regex CharRegex("^(u8|u|U|L)?");
llvm::StringMap<StringRef> CharPrefix({
  {"char", ""},
  {"char8_t", "u8"},
  {"char16_t", "u"},
  {"char32_t", "U"},
  {"wchar_t", "L"},
});

llvm::Regex IntRegex(
  "(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?)|([uU]?[zZ]?)|([zZ]?[uU]?))?$");
llvm::StringMap<StringRef> IntSuffix({
  {"int", ""},
  {"unsigned int", "u"},
  {"long", "l"},
  {"unsigned long", "ul"},
  {"long long", "ll"},
  {"unsigned long long", "ull"},
  {"size_t", "uz"},
  {"std::size_t", "uz"},
});

llvm::Regex FloatRegex(
  "([fF]|[lL]|([fF]16)|([fF]32)|([fF]64)|([fF]128)|((bf|BF)16))?$");
llvm::StringMap<StringRef> FloatSuffix({
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
});

} // namespace

void UseBuiltinLiteralsCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      explicitCastExpr(
          (has(ignoringParenImpCasts(anyOf(
              characterLiteral().bind("char"),
			  integerLiteral().bind("int"),
              floatLiteral().bind("float"))))))
          .bind("expr"),
      this);
  Finder->addMatcher(
      explicitCastExpr(
          has(initListExpr(has(ignoringParenImpCasts(anyOf(
              characterLiteral().bind("char"),
			  integerLiteral().bind("int"),
              floatLiteral().bind("float")))))))
          .bind("expr"),
      this);
}

void UseBuiltinLiteralsCheck::check(const MatchFinder::MatchResult &Result) {
  
  const SourceManager &Sources = *Result.SourceManager;
  const auto *MatchedCast = Result.Nodes.getNodeAs<ExplicitCastExpr>("expr");
  assert(MatchedCast);
  std::string CastTypeStr = MatchedCast->getType().getAsString();

  std::string Fix;

  if (const auto *CharLit = Result.Nodes.getNodeAs<CharacterLiteral>("char");
      CharLit && CharPrefix.contains(CastTypeStr)) {

    StringRef LitText =
        getRawStringRef(CharLit->getLocation(), Sources, getLangOpts());

    Fix.append(CharPrefix[CastTypeStr]);
    Fix.append(CharRegex.sub("", LitText.str()));

    diag(MatchedCast->getExprLoc(), "use builtin literals instead of casts")
	  << FixItHint::CreateReplacement(MatchedCast->getSourceRange(), Fix.c_str());
  } else
  if (const auto *IntLit = Result.Nodes.getNodeAs<IntegerLiteral>("int");
      IntLit && IntSuffix.contains(CastTypeStr)) {

    StringRef LitText =
        getRawStringRef(IntLit->getLocation(), Sources, getLangOpts());

    Fix.append(IntRegex.sub("", LitText.str()));
    Fix.append(IntSuffix[CastTypeStr]);

    diag(MatchedCast->getExprLoc(), "use builtin literals instead of casts")
	  << FixItHint::CreateReplacement(MatchedCast->getSourceRange(), Fix.c_str());
  } else
  if (const auto *FloatLit = Result.Nodes.getNodeAs<FloatingLiteral>("float");
      FloatLit && FloatSuffix.contains(CastTypeStr)) {

    StringRef LitText =
        getRawStringRef(FloatLit->getLocation(), Sources, getLangOpts());

    Fix.append(FloatRegex.sub("", LitText.str()));
    Fix.append(FloatSuffix[CastTypeStr]);

    diag(MatchedCast->getExprLoc(), "use builtin literals instead of casts")
	  << FixItHint::CreateReplacement(MatchedCast->getSourceRange(), Fix.c_str());
  }
}

} // namespace clang::tidy::readability
