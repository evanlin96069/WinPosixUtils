#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "wopt.h"

void stime_arg(WCHAR*, FILETIME*);
void stime_file(WCHAR*, FILETIME*);

USAGE("usage: touch [-acm] [-r file] [-t [[CC]YY]MMDDhhmm[.SS]] file ...");

int wmain(int argc, wchar_t* argv[]) {
    FILETIME ft[2];
    int aflag, cflag, mflag, ch, timeset;

    aflag = cflag = mflag = timeset = 0;

    GetSystemTimeAsFileTime(&ft[0]);

    FOR_OPTS(argc, argv, {
        case 'a':
            aflag = 1;
            break;
        case 'c':
            cflag = 1;
            break;
        case 'm':
            mflag = 1;
            break;
        case 'r':
            timeset = 1;
            stime_file(OPTARG(argc, argv), ft);
            break;
        case 't':
            timeset = 1;
            stime_arg(OPTARG(argc, argv), ft);
            break;
    });

    if (aflag == 0 && mflag == 0)
        aflag = mflag = 1;

    if (!timeset)
        ft[1] = ft[0];

    if (*argv == NULL) {
        fprintf(stderr, "missing file operand\n");
        usage();
    }

    for (int rval = 0; *argv; ++argv) {
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
            WCHAR err[256] = {0};
            FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                           MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), err, 255,
                           NULL);
            fwprintf(stderr, L"cannot touch file -- '%s': %s", *argv, err);
            continue;
        }

        SetFileTime(handle, NULL, aflag ? &ft[0] : NULL, mflag ? &ft[1] : NULL);
        CloseHandle(handle);
    }

    return 0;
}

#define ATOI2(ar)                           \
    ((ar)[0] - '0') * 10 + ((ar)[1] - '0'); \
    (ar) += 2;

void stime_arg(WCHAR* arg, FILETIME* ft) {
    WCHAR* p;
    SYSTEMTIME t;
    GetSystemTime(&t);
    t.wMilliseconds = 0;
    if ((p = wcschr(arg, '.')) == NULL) {
        t.wSecond = 0;
    } else {
        if (wcslen(p + 1) != 2)
            goto err;
        *p++ = '\0';
        t.wSecond = ATOI2(p);
    }

    int yearset = 0;
    switch (wcslen(arg)) {
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
    fprintf(
        stderr,
        "out of range or illegal time specification: [[CC]YY]MMDDhhmm[.SS]\n");
    exit(1);
}

void stime_file(WCHAR* fname, FILETIME* ft) {
    HANDLE handle = CreateFileW(fname, GENERIC_READ, 0, NULL, OPEN_EXISTING,
                                FILE_FLAG_BACKUP_SEMANTICS, NULL);
    if (handle == INVALID_HANDLE_VALUE) {
        WCHAR err[256] = {0};
        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(),
                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), err, 255,
                       NULL);
        fwprintf(stderr, L"failed to get attributes of '%s': %s", fname, err);
        exit(1);
    }
    GetFileTime(handle, NULL, &ft[0], &ft[1]);
    CloseHandle(handle);
}
