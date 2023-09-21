/* This file is dedicated to the public domain. */

#ifndef OPT_H
#define OPT_H

#include <winio.h>

/*
 * Prints usage information and exits with a given return code.
 */
static inline void usagex(int ret);

/*
 * Prints usage information and exits with the status code 1.
 */
static inline void usage(void);

/*
 * Defines the usage string of the program. Must be specified before main() if
 * FOR_OPTS is used to parse arguments.
 */
#define USAGE(s)                                  \
    static const WCHAR* _usage;                   \
    static inline void usagex(int ret) {          \
        io_puts(STDERR, _usage);                  \
        io_putc(STDERR, L'\n');                   \
        ExitProcess(ret);                         \
    }                                             \
    static inline void usage(void) { usagex(1); } \
    static const WCHAR* _usage = L"" s

static inline void _opt_bad(WCHAR* s, WCHAR c) {
    io_puts(STDERR, s);
    io_puts(STDERR, L" -- '");
    io_putc(STDERR, c);
    io_puts(STDERR, L"'\n");
    usage();
}

/*
 * Parses POSIX-compliant command-line options. Takes the names of argc and argv
 * plus a braced code block containing switch cases matching each option
 * character. The default case is already provided and causes an error message
 * to be printed and the program to exit.
 */
#define FOR_OPTS(argc, argv, CODE)                                           \
    do {                                                                     \
        --(argc);                                                            \
        ++(argv);                                                            \
        for (WCHAR* _p = *(argv), *_p1; *(argv); _p = *++(argv), --(argc)) { \
            (void)(_p1); /* avoid unused warnings if OPTARG isn't used */    \
            if (*_p != L'-' || !*++_p)                                       \
                break;                                                       \
            if (*_p == L'-' && !*(_p + 1)) {                                 \
                ++(argv);                                                    \
                --(argc);                                                    \
                break;                                                       \
            }                                                                \
            while (*_p) {                                                    \
                switch (*_p++) {                                             \
                    CODE default : _opt_bad(L"invalid option", *(_p - 1));   \
                }                                                            \
            }                                                                \
        }                                                                    \
    } while (0)

/*
 * Produces the string given as a parameter to the current option - must only be
 * used inside one of the cases given to FOR_OPTS. If there is no parameter
 * given, prints an error message and exits.
 */
#define OPTARG(argc, argv)                                              \
    (*_p ? (_p1 = _p, _p = L"", _p1)                                    \
         : (*(--(argc), ++(argv))                                       \
                ? *(argv)                                               \
                : (_opt_bad(L"option requires an argument", *(_p - 1)), \
                   (WCHAR*)0)))

#endif
