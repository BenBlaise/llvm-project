// RUN: %check_clang_tidy %s readability-use-builtin-literals %t

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

reinterpret_cast<int>(1);
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: 1;

}

#define OPSHIFT ((unsigned)27)
#define OCHAR (2LU<<OPSHIFT)
#define OPSHIFT2 (27)
#define OCHAR2 (2LU<<(unsigned)OPSHIFT2)

void warn_and_recommend_fix() {

OCHAR;
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin 'u' instead of cast to 'unsigned int' [readability-use-builtin-literals]
OCHAR2;
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin 'u' instead of cast to 'unsigned int' [readability-use-builtin-literals]
}

#define INT_MAX 2147483647
#define MAXCOL 2

template <typename T>
T f() {
  return T(1);
}

int no_warn() {

(void)0;
(unsigned*)0;


static_cast<unsigned>(INT_MAX);
(unsigned)MAXCOL;
return f<int>();
}
