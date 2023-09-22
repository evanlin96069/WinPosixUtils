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

#define BUFMAX 1024
static void io_write(DWORD fd, const WCHAR* buf, DWORD len) {
    DWORD tmp;
    HANDLE handle =
        GetStdHandle(fd == STDOUT ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE);
    if (GetConsoleMode(handle, &tmp)) {
        WriteConsoleW(handle, buf, len, &tmp, 0);
        return;
    }

    // GetConsoleMode might fail when redirecting to a file.
    char u8[BUFMAX + 1];
    u8[BUFMAX] = '\0';
    int ulen =
        WideCharToMultiByte(CP_UTF8, 0, buf, len, u8, BUFMAX, NULL, NULL);
    WriteFile(handle, u8, ulen, &tmp, 0);
}

static inline void io_putc(DWORD fd, WCHAR c) { io_write(fd, &c, 1); }

static inline void io_puts(DWORD fd, const WCHAR* buf) {
    io_write(fd, buf, wstrlen(buf));
}

#endif
