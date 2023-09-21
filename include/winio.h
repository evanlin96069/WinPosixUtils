#ifndef WINIO_H
#define WINIO_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#define STDOUT 1
#define STDERR 2

static inline DWORD wstrlen(const WCHAR* str) {
    DWORD len = 0;
    while (*str) {
        len++;
        str++;
    }
    return len;
}

static void io_write(DWORD fd, const WCHAR* buf, DWORD len) {
    static HANDLE hStdout = INVALID_HANDLE_VALUE;
    static HANDLE hStderr = INVALID_HANDLE_VALUE;
    DWORD tmp;
    if (hStdout == INVALID_HANDLE_VALUE)
        hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStderr == INVALID_HANDLE_VALUE)
        hStderr = GetStdHandle(STD_ERROR_HANDLE);

    if (fd == STDOUT)
        WriteConsoleW(hStdout, buf, len, &tmp, 0);
    else if (fd == STDERR)
        WriteConsoleW(hStdout, buf, len, &tmp, 0);
}

static inline void io_putc(DWORD fd, WCHAR c) { io_write(fd, &c, 1); }

static inline void io_puts(DWORD fd, const WCHAR* buf) {
    io_write(fd, buf, wstrlen(buf));
}

#endif
