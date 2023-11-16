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

typedef bool (*RuleOnStd)(const LangStandard &LS);

struct Replacement {
  Replacement(const StringRef &_Seq, const RuleOnStd _Std = NULL)
      : Seq(_Seq), Std(_Std) {}
  bool operator()(const LangOptions &LO) const {
    return Std ? Std(LangStandard::getLangStandardForKind(LO.LangStd)) : true;
  }
  StringRef Seq;
  RuleOnStd Std;
};

llvm::Regex CharRegex("^(u8|u|U|L)?");
llvm::StringMap<Replacement> CharPrefix({
    {"char", {""}},
    {"char8_t", {"u8"}},
    {"char16_t", {"u"}},
    {"char32_t", {"U"}},
    {"wchar_t", {"L"}},
});

llvm::Regex
    IntRegex("(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?)|([uU]?[zZ]?)|([zZ]?[uU]?))?$");
llvm::StringMap<Replacement> IntSuffix({
    {"int", {""}},
    {"unsigned int", {"u"}},
    {"long", {"L"}},
    {"unsigned long", {"uL"}},
    {"long long", {"LL"}},
    {"unsigned long long", {"uLL"}},
    {"size_t", {"uz", [](const auto &LS) { return LS.isCPlusPlus23(); }}},
    {"std::size_t", {"uz", [](const auto &LS) { return LS.isCPlusPlus23(); }}},
});

llvm::Regex FloatRegex(
    "([fF]|[lL]|([fF]16)|([fF]32)|([fF]64)|([fF]128)|((bf|BF)16))?$");
llvm::StringMap<Replacement> FloatSuffix({
    {"double", {""}},
    {"float", {"f"}},
    {"long double", {"L"}},
    {"std::float16_t", {"f16"}},
    {"std::float32_t", {"f32"}},
    {"std::float64_t", {"f64"}},
    {"std::float128_t", {"f128"}},
    {"std::bfloat16_t", {"bf16"}},
    {"float16_t", {"f16"}},
    {"float32_t", {"f32"}},
    {"float64_t", {"f64"}},
    {"float128_t", {"f128"}},
    {"bfloat16_t", {"bf16"}},
});

} // namespace

void UseBuiltinLiteralsCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      traverse(TK_IgnoreUnlessSpelledInSource,
               explicitCastExpr((has(ignoringParenImpCasts(
                                    anyOf(characterLiteral().bind("char"),
                                          integerLiteral().bind("int"),
                                          floatLiteral().bind("float"))))))
                   .bind("expr")),
      this);
  Finder->addMatcher(
      traverse(TK_IgnoreUnlessSpelledInSource,
               explicitCastExpr(has(initListExpr(has(ignoringParenImpCasts(
                                    anyOf(characterLiteral().bind("char"),
                                          integerLiteral().bind("int"),
                                          floatLiteral().bind("float")))))))
                   .bind("expr")),
      this);
}

void UseBuiltinLiteralsCheck::check(const MatchFinder::MatchResult &Result) {

  const auto &Sources = *Result.SourceManager;
  const auto &Nodes = Result.Nodes;

  const auto *MatchedCast = Nodes.getNodeAs<ExplicitCastExpr>("expr");
  assert(MatchedCast);

  std::string CastType = MatchedCast->getTypeAsWritten().getAsString();
  std::string Fix; // Replacement string for the fix-it hint.
  StringRef Seq;   // Literal sequence, prefix or suffix.

  if (const auto *CharLit = Nodes.getNodeAs<CharacterLiteral>("char");
      CharLit && CharPrefix.contains(CastType)) {
    if (const auto &Rep = CharPrefix.at(CastType); Rep(getLangOpts())) {

      StringRef LitText =
          getRawStringRef(CharLit->getLocation(), Sources, getLangOpts());

      Seq = Rep.Seq;
      if (!CharLit->getLocation().isMacroID()) {
        Fix.append(Rep.Seq);
        Fix.append(CharRegex.sub("", LitText.str()));
      }
    }
  } else if (const auto *IntLit = Nodes.getNodeAs<IntegerLiteral>("int");
             IntLit && IntSuffix.contains(CastType)) {
    if (const auto &Rep = IntSuffix.at(CastType); Rep(getLangOpts())) {

      StringRef LitText =
          getRawStringRef(IntLit->getLocation(), Sources, getLangOpts());

      Seq = Rep.Seq;
      if (!IntLit->getLocation().isMacroID()) {
        Fix.append(IntRegex.sub("", LitText.str()));
        Fix.append(Rep.Seq);
      }
    }
  } else if (const auto *FloatLit = Nodes.getNodeAs<FloatingLiteral>("float");
             FloatLit && FloatSuffix.contains(CastType)) {
    if (const auto &Rep = FloatSuffix.at(CastType); Rep(getLangOpts())) {

      StringRef LitText =
          getRawStringRef(FloatLit->getLocation(), Sources, getLangOpts());

      Seq = Rep.Seq;
      if (!FloatLit->getLocation().isMacroID()) {
        Fix.append(FloatRegex.sub("", LitText.str()));
        Fix.append(Rep.Seq);
      }
    }
  }

  const TypeLoc CastTypeLoc = MatchedCast->getTypeInfoAsWritten()->getTypeLoc();

  if (!Fix.empty() && !CastTypeLoc.getBeginLoc().isMacroID()) {

    // Recommend fix-it when no part of the explicit cast comes from a macro.
    diag(MatchedCast->getExprLoc(), "use builtin literals instead of casts")
        << FixItHint::CreateReplacement(MatchedCast->getSourceRange(),
                                        Fix.c_str());
  } else if (!Seq.empty() && MatchedCast->getExprLoc().isMacroID()) {

    // Recommend manual fix when the entire explicit cast is within a macro.
    diag(MatchedCast->getExprLoc(), "use builtin '%0' instead of cast to '%1'")
        << Seq.str() << CastType;
  }
}

} // namespace clang::tidy::readability
