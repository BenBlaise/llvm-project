// RUN: %check_clang_tidy -std=c++23-or-later %s readability-use-builtin-literals %t

#include <stddef.h>

void warn_and_fix() {

(size_t)6zu;
// CHECK-MESSAGES: :[[@LINE-1]]:1: warning: use builtin literals instead of casts [readability-use-builtin-literals]
// CHECK-FIXES: 6uz;

}
