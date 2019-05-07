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
 * Note that "assert.h" may be included more that once in a program with
 * different setting of NDEBUG. Hence the slightly unusual first-time
 * only flag.
 */

#ifndef __assert_h
#define __assert_h

#undef  __CLIBNS
#ifdef  __cplusplus
#define __CLIBNS ::std::
#else
#define __CLIBNS
#endif

#ifdef __cplusplus
extern "C" {
#endif

extern void abort(void);
extern void cm_backtrace_assert(const char *expr, const char *file, unsigned line);

#ifdef __cplusplus
}   /* extern "C" */
#endif

#else   /* __assert_h */
#undef assert
#endif  /* __assert_h */


#ifdef NDEBUG
#define assert(e)  ((void)0)
#else
#define assert(e)  ((e) ? (void)0 : (void)(cm_backtrace_assert(#e, __FILE__, __LINE__), __CLIBNS abort()))
#endif

/* end of assert.h */
