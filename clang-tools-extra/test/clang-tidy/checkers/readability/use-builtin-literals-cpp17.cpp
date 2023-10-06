// RUN: %check_clang_tidy -std=c++17-or-later %s readability-use-builtin-literals %t

void warn_and_fix() {

(char)u8'a';
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: 'a';
(wchar_t)u8'a';
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: L'a';

}

