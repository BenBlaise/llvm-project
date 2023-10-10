// RUN: %check_clang_tidy -std=c++20 %s readability-use-builtin-literals %t

void warn_and_fix() {

(char16_t)U'a';
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: u'a';
(char32_t)u'a';
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: U'a';

(int)1;
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: 1;
(unsigned int)0x1ul;
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: 0x1u;
(long int)2l;
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: 2l;
(unsigned long int)0x2lu;
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: 0x2ul;
(long long int)3ll;
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: 3ll;
(unsigned long long int)0x3llu;
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: 0x3ull;

(double)1.f;
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: 1.;
(float)2.;
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: 2.f;
(long double)3e0f;
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: 3e0l;

float(2.);
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: 2.f;
double{2.};
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: 2.;

}

