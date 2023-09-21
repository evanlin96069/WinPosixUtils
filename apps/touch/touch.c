#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#if defined(_MSC_VER)
#pragma comment(linker, "/subsystem:console")
#pragma comment(lib, "kernel32")
#pragma comment(lib, "Shell32")
#endif

#include "winio.h"
#include "opt.h"

static inline WCHAR* wstrchr(WCHAR* str, WCHAR c) {
    while (*str) {
        if (*str == c)
            return (WCHAR*)str;
        str++;
    }
    return NULL;
}

static void stime_arg(WCHAR*, FILETIME*);
static void stime_file(WCHAR*, FILETIME*);
static void inline print_last_err(const WCHAR*, const WCHAR*);

USAGE(L"usage: touch [-acm] [-r file] [-t [[CC]YY]MMDDhhmm[.SS]] file ...");

int mainCRTStartup(void) {
    int argc;
    WCHAR** argv;

    WCHAR* cmd = GetCommandLineW();
    argv = CommandLineToArgvW(cmd, &argc);
    if (!argv) {
        io_puts(STDERR, L"failed to get command line\n");
        ExitProcess(1);
    }

    FILETIME ft[2];
    int aflag, cflag, mflag, timeset;

    aflag = cflag = mflag = timeset = 0;

    GetSystemTimeAsFileTime(&ft[0]);

    FOR_OPTS(argc, argv, {
        case L'a':
            aflag = 1;
            break;
        case L'c':
            cflag = 1;
            break;
        case L'm':
            mflag = 1;
            break;
        case L'r':
            timeset = 1;
            stime_file(OPTARG(argc, argv), ft);
            break;
        case L't':
            timeset = 1;
            stime_arg(OPTARG(argc, argv), ft);
            break;
    });

    if (aflag == 0 && mflag == 0)
        aflag = mflag = 1;

    if (!timeset)
        ft[1] = ft[0];

    if (*argv == NULL) {
        io_puts(STDERR, L"missing file operand\n");
        usage();
    }

    for (; *argv; ++argv) {
        HANDLE handle = INVALID_HANDLE_VALUE;
        if (GetFileAttributesW(*argv) == INVALID_FILE_ATTRIBUTES) {
            if (!cflag)
                handle = CreateFileW(*argv, GENERIC_WRITE, 0, NULL, CREATE_NEW,
                                     FILE_ATTRIBUTE_NORMAL, NULL);
            else
                continue;
        } else {
            handle = CreateFileW(*argv, GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
                                 FILE_FLAG_BACKUP_SEMANTICS, NULL);
        }

        if (handle == INVALID_HANDLE_VALUE) {
            print_last_err(L"cannot touch file", *argv);
            continue;
        }

        SetFileTime(handle, NULL, aflag ? &ft[0] : NULL, mflag ? &ft[1] : NULL);
        CloseHandle(handle);
    }

    return 0;
}

#define ATOI2(ar)                             \
    ((ar)[0] - L'0') * 10 + ((ar)[1] - L'0'); \
    (ar) += 2;

static void stime_arg(WCHAR* arg, FILETIME* ft) {
    WCHAR* p;
    SYSTEMTIME t;
    GetSystemTime(&t);
    t.wMilliseconds = 0;
    if ((p = wstrchr(arg, L'.')) == NULL) {
        t.wSecond = 0;
    } else {
        if (wstrlen(p + 1) != 2)
            goto err;
        *p++ = L'\0';
        t.wSecond = ATOI2(p);
    }

    WORD yearset = 0;
    switch (wstrlen(arg)) {
        case 12:  // CCYYMMDDhhmm
            t.wYear = ATOI2(arg);
            t.wYear *= 100;
            yearset = 1;
            // fall through
        case 10:  // YYMMDDhhmm
            if (yearset) {
                yearset = ATOI2(arg);
                t.wYear += yearset;
            } else {
                yearset = ATOI2(arg);
                if (yearset < 69)
                    t.wYear = yearset + 2000;
                else
                    t.wYear = yearset + 1900;
            }
            // fall through
        case 8:  // MMDDhhmm
            t.wMonth = ATOI2(arg);
            t.wDay = ATOI2(arg);
            t.wHour = ATOI2(arg);
            t.wMinute = ATOI2(arg);
            break;
        default:
            goto err;
    }

    if (TzSpecificLocalTimeToSystemTime(NULL, &t, &t) == 0 ||
        SystemTimeToFileTime(&t, &ft[0]) == 0)
        goto err;

    ft[1] = ft[0];

    return;

err:
    io_puts(
        STDERR,
        L"out of range or illegal time specification: [[CC]YY]MMDDhhmm[.SS]\n");
    ExitProcess(1);
}

static void stime_file(WCHAR* fname, FILETIME* ft) {
    HANDLE handle = CreateFileW(fname, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                                FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        print_last_err(L"failed to get attributes of", fname);
        ExitProcess(1);
    }
    GetFileTime(handle, NULL, &ft[0], &ft[1]);
    CloseHandle(handle);
}

static inline void print_last_err(const WCHAR* msg, const WCHAR* name) {
    WCHAR err[256];
    err[255] = L'\0';
    FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), err, 255, NULL);
    io_puts(STDERR, msg);
    io_puts(STDERR, L" '");
    io_puts(STDERR, name);
    io_puts(STDERR, L"': ");
    io_puts(STDERR, err);
}
