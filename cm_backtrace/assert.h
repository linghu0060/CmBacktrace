/* assert.h: ANSI 'C' (X3J11 Oct 88) library header section 4.2 */
/* Copyright (C) Codemist Ltd., 1988-1993                       */

/*
 * The assert macro puts diagnostics into programs. When it is executed,
 * if its argument expression is false, it writes information about the
 * call that failed (including the text of the argument, the name of the
 * source file, and the source line number - the latter are respectively
 * the values of the preprocessing macros __FILE__ and __LINE__) on the
 * standard error stream. It then calls the abort function.
 * If its argument expression is true, the assert macro returns no value.
 */

/*
 * Note that <assert.h> may be included more that once in a program with
 * different setting of NDEBUG. Hence the slightly unusual first-time
 * only flag.
 */

#include "stdlib.h"

extern void cm_backtrace_assert(const char *expr, const char *file, size_t line);

#ifndef __assert_h
#   define __assert_h
#   ifdef __cplusplus
        namespace std {
#           define __CLIBNS ::std::
            extern "C" {
#   else
#       define __CLIBNS
#   endif  /* __cplusplus */
#   ifdef __cplusplus
            }  /* extern "C" */
        }  /* namespace std */
#   endif
#   ifdef NDEBUG
#       define assert(e)  ((void)0)
#   else
#       define assert(e)  ((e) ? (void)0 : (__CLIBNS cm_backtrace_assert(#e, __FILE__, __LINE__), __CLIBNS abort()))
#   endif
#endif

