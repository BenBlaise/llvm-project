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

typedef bool(*RuleOnStd)(const LangStandard &LS);

struct Replacement
{
	Replacement(const StringRef& _Seq, const RuleOnStd _Std = NULL)
		: Seq(_Seq), Std(_Std) {}
	bool operator()(const LangOptions &LO) const {
	  return Std ? Std(LangStandard::getLangStandardForKind(LO.LangStd)) : true;
	}
	StringRef Seq;
	RuleOnStd Std;
};

llvm::Regex CharRegex("^(u8|u|U|L)?");
llvm::StringMap<Replacement> CharPrefix({
  { "char", { "" } },
  { "char8_t", { "u8" } },
  { "char16_t", { "u" } },
  { "char32_t", { "U" } },
  { "wchar_t", { "L" } },
});

llvm::Regex IntRegex(
  "(([uU]?[lL]{0,2})|([lL]{0,2}[uU]?)|([uU]?[zZ]?)|([zZ]?[uU]?))?$");
llvm::StringMap<Replacement> IntSuffix({
  { "int", { "" } },
  { "unsigned int", { "u" } },
  { "long", { "l" } },
  { "unsigned long", { "ul" } },
  { "long long", { "ll" } },
  { "unsigned long long", { "ull" } },
  { "size_t", { "uz", [](const auto &LS){ return LS.isCPlusPlus23(); } } },
  { "std::size_t", { "uz", [](const auto &LS){ return LS.isCPlusPlus23(); } } },
});

llvm::Regex FloatRegex(
  "([fF]|[lL]|([fF]16)|([fF]32)|([fF]64)|([fF]128)|((bf|BF)16))?$");
llvm::StringMap<Replacement> FloatSuffix({
  { "double", { "" } },
  { "float", { "f" } },
  { "long double", { "l" } },
  { "std::float16_t", { "f16" } },
  { "std::float32_t", { "f32" } },
  { "std::float64_t", { "f64" } },
  { "std::float128_t", { "f128" } },
  { "std::bfloat16_t", { "bf16" } },
  { "float16_t", { "f16" } },
  { "float32_t", { "f32" } },
  { "float64_t", { "f64" } },
  { "float128_t", { "f128" } },
  { "bfloat16_t", { "bf16" } },
});

} // namespace

void UseBuiltinLiteralsCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(traverse(TK_IgnoreUnlessSpelledInSource,
      explicitCastExpr(
          (has(ignoringParenImpCasts(anyOf(
              characterLiteral().bind("char"),
			  integerLiteral().bind("int"),
              floatLiteral().bind("float"))))))
          .bind("expr")
  ), this);
  Finder->addMatcher(traverse(TK_IgnoreUnlessSpelledInSource,
      explicitCastExpr(
          has(initListExpr(has(ignoringParenImpCasts(anyOf(
              characterLiteral().bind("char"),
			  integerLiteral().bind("int"),
              floatLiteral().bind("float")))))))
          .bind("expr")
  ), this);
}

void UseBuiltinLiteralsCheck::check(const MatchFinder::MatchResult &Result) {
  
  const SourceManager &Sources = *Result.SourceManager;

  const auto *MatchedCast = Result.Nodes.getNodeAs<ExplicitCastExpr>("expr");
  assert(MatchedCast);
  std::string CastTypeStr = MatchedCast->getTypeAsWritten().getAsString();

  std::string Fix;
  StringRef Seq;

  if (const auto *CharLit = Result.Nodes.getNodeAs<CharacterLiteral>("char");
      CharLit && CharPrefix.contains(CastTypeStr)) {
	if(const auto &Replace = CharPrefix.at(CastTypeStr); Replace(getLangOpts())){

      StringRef LitText =
        getRawStringRef(CharLit->getLocation(), Sources, getLangOpts());

	  Seq = Replace.Seq;
	  if(!CharLit->getLocation().isMacroID()) {
        Fix.append(Replace.Seq);
        Fix.append(CharRegex.sub("", LitText.str()));
	  }
	}
  } else
  if (const auto *IntLit = Result.Nodes.getNodeAs<IntegerLiteral>("int");
      IntLit && IntSuffix.contains(CastTypeStr)) {
	if(const auto &Replace = IntSuffix.at(CastTypeStr); Replace(getLangOpts())){

      StringRef LitText =
        getRawStringRef(IntLit->getLocation(), Sources, getLangOpts());

	  Seq = Replace.Seq;
	  if(!IntLit->getLocation().isMacroID()) {
        Fix.append(IntRegex.sub("", LitText.str()));
        Fix.append(Replace.Seq);
	  }
	}
  } else
  if (const auto *FloatLit = Result.Nodes.getNodeAs<FloatingLiteral>("float");
      FloatLit && FloatSuffix.contains(CastTypeStr)) {
	if(const auto &Replace = FloatSuffix.at(CastTypeStr); Replace(getLangOpts())){

      StringRef LitText =
        getRawStringRef(FloatLit->getLocation(), Sources, getLangOpts());

	  Seq = Replace.Seq;
	  if(!FloatLit->getLocation().isMacroID()) {
        Fix.append(FloatRegex.sub("", LitText.str()));
        Fix.append(Replace.Seq);
	  }
	}
  }

  if(!Fix.empty()) {
    diag(MatchedCast->getExprLoc(), "use builtin literals instead of casts")
	  << FixItHint::CreateReplacement(MatchedCast->getSourceRange(), Fix.c_str());
  } else if(!Seq.empty() && MatchedCast->getExprLoc().isMacroID()) {
    diag(MatchedCast->getExprLoc(), "use builtin '%0' instead of cast to '%1'")
	  << Seq.str() << CastTypeStr;
  }
}

} // namespace clang::tidy::readability
