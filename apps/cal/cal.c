#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>

#if defined(_MSC_VER)
#pragma comment(linker, "/subsystem:console")
#pragma comment(lib, "kernel32")
#pragma comment(lib, "Shell32")
#endif

#include "winio.h"

static int watoi(const WCHAR* str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    while (str[i] == L' ' || str[i] == L'\t') {
        i++;
    }

    if (str[i] == L'-' || str[i] == L'+') {
        sign = (str[i++] == L'-') ? -1 : 1;
    }

    while (str[i] >= L'0' && str[i] <= L'9') {
        result = result * 10 + (str[i] - L'0');
        i++;
    }

    return sign * result;
}

static inline void usage(void) {
    io_puts(STDERR, L"usage: cal [[month] year]\n");
    ExitProcess(1);
}

static const WCHAR month_str[12][10] = {
    L"January", L"February", L"March",     L"April",   L"May",      L"June",
    L"July",    L"August",   L"September", L"October", L"November", L"December",
};

static const int month_str_len[12] = {
    7, 8, 5, 5, 3, 4, 4, 6, 9, 7, 8, 8,
};

static const int month_len[12] = {
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
};

static void print_month(int, int);
static void print_year(int);

int mainCRTStartup(void) {
    int argc;
    WCHAR** argv;

    WCHAR* cmd = GetCommandLineW();
    argv = CommandLineToArgvW(cmd, &argc);
    if (!argv) {
        io_puts(STDERR, L"failed to get command line\n");
        ExitProcess(1);
    }

    int year = 0;
    int month = 0;
    int monthset = 1;
    switch (argc) {
        case 1: {
            SYSTEMTIME lt;
            GetLocalTime(&lt);
            year = lt.wYear;
            month = lt.wMonth;
        } break;
        case 2:
            year = watoi(argv[1]);
            monthset = 0;
            break;
        case 3:
            month = watoi(argv[1]);
            year = watoi(argv[2]);
            break;
        default:
            usage();
    }

    if (year < 1 || year > 9999) {
        io_puts(STDERR, L"year not in range 1..9999\n");
        ExitProcess(1);
    }

    if (monthset && (month < 1 || month > 12)) {
        io_puts(STDERR, L"month not in range 1..12");
        ExitProcess(1);
    }

    if (monthset) {
        print_month(year, month);
    } else {
        print_year(year);
    }

    return 0;
}

struct Buffer {
    WCHAR* buf;
    DWORD len;
};

static inline int digit_len(int n) {
    if (n > 999)
        return 4;
    if (n > 99)
        return 3;
    if (n > 9)
        return 2;
    return 1;
}

static inline void buf_append_char(struct Buffer* buf, WCHAR c) {
    buf->buf[buf->len++] = c;
}

static inline void buf_append_year(struct Buffer* buf, int year) {
    switch (digit_len(year)) {
        case 4:
            buf_append_char(buf, L'0' + (WCHAR)(year / 1000));
            // fall through
        case 3:
            buf_append_char(buf, L'0' + year / 100 % 10);
            // fall through
        case 2:
            buf_append_char(buf, L'0' + year / 10 % 10);
            // fall through
        case 1:
            buf_append_char(buf, L'0' + year % 10);
    }
}

static inline void buf_append_month(struct Buffer* buf, int month) {
    for (int i = 0; i < month_str_len[month - 1]; i++) {
        buf_append_char(buf, month_str[month - 1][i]);
    }
}

static inline void buf_append_day(struct Buffer* buf, int day) {
    buf_append_char(buf, day < 10 ? L' ' : L'0' + (WCHAR)(day / 10));
    buf_append_char(buf, L'0' + (day % 10));
}

static inline void buf_append_space(struct Buffer* buf, int n) {
    for (int i = 0; i < n; i++) {
        buf_append_char(buf, L' ');
    }
}
#define CAL_WIDTH 20
#define CAL_PADDING 2
#define BUFFER_SIZE(buf) (sizeof(buf) / sizeof(buf[0]))

static void print_year_header(int year) {
    WCHAR data[((CAL_WIDTH + CAL_PADDING) * 3) / 2];
    struct Buffer header = {data, 0};
    buf_append_space(&header, BUFFER_SIZE(data));
    int y_len = digit_len(year);
    header.len -= y_len;
    buf_append_year(&header, year);
    io_write(STDOUT, header.buf, header.len);
    io_putc(STDOUT, L'\n');
}

static void put_month_header(struct Buffer* buf, int month) {
    int m_len = month_str_len[month - 1];
    int left_pad = (CAL_WIDTH - m_len) / 2;
    int right_pad = CAL_WIDTH - left_pad - m_len + CAL_PADDING;
    buf_append_space(buf, left_pad);
    buf_append_month(buf, month);
    buf_append_space(buf, right_pad);
}

static void put_month_with_year_header(struct Buffer* buf, int month,
                                       int year) {
    int m_len = month_str_len[month - 1];
    int y_len = digit_len(year);
    int left_pad = (CAL_WIDTH - m_len - y_len - 1) / 2;
    int right_pad = CAL_WIDTH - left_pad - m_len - 1 - y_len + CAL_PADDING;
    buf_append_space(buf, left_pad);
    buf_append_month(buf, month);
    buf_append_space(buf, 1);
    buf_append_year(buf, year);
    buf_append_space(buf, right_pad);
}

static inline int is_leap(int year) {
    if (year <= 1752) {
        return year % 4 == 0;
    }
    return year % 4 == 0 && (year % 100 != 0 || year % 400 == 0);
}

static inline int get_month_len(int year, int month) {
    return (month == 2 && is_leap(year)) ? 29 : month_len[month - 1];
}

static inline int is_gregorian(int year, int month, int day) {
    return year > 1752 || (year == 1752 && month > 9) ||
           (year == 1752 && month == 9 && day > 2);
}

static int zeller(int year, int month, int day) {
    int y, c, m, era;
    year -= (month + 21) / 12 % 2;
    y = year % 100;
    c = year / 100 % 100;
    m = 3 + (9 + month) % 12;
    era = is_gregorian(year, month, day) ? c / 4 - 2 * c : 5 - c;
    return (day + 13 * (m + 1) / 5 + y + y / 4 + era + 52 * 7 + 6) % 7 + 1;
}

static void put_month(struct Buffer* pbuf[8], int year, int month,
                      int year_header) {
    if (year_header) {
        put_month_with_year_header(pbuf[0], month, year);
    } else {
        put_month_header(pbuf[0], month);
    }

    const WCHAR day_str[] = L"Su Mo Tu We Th Fr Sa";
    for (int i = 0; i < BUFFER_SIZE(day_str) - 1; i++) {
        buf_append_char(pbuf[1], day_str[i]);
    }
    buf_append_space(pbuf[1], CAL_PADDING);

    int m_len = get_month_len(year, month);
    int day = 1;
    int d1 = zeller(year, month, 1);
    for (int i = 2; i < 8; i++) {
        for (int j = 1; j <= 7; j++) {
            if ((day > m_len) || (i == 2 && j < d1)) {
                buf_append_char(pbuf[i], L' ');
                buf_append_char(pbuf[i], L' ');
            } else {
                if (year == 1752 && month == 9 && day == 3)
                    day += 11;
                buf_append_day(pbuf[i], day);
                day++;
            }
            buf_append_char(pbuf[i], L' ');
        }
        buf_append_space(pbuf[i], CAL_PADDING - 1);
    }
}

static void print_month(int year, int month) {
    WCHAR data[8][CAL_WIDTH + CAL_PADDING];
    struct Buffer buf[8];
    struct Buffer* pbuf[8];
    for (int i = 0; i < 8; i++) {
        buf[i].buf = (WCHAR*)&data[i];
        buf[i].len = 0;
        pbuf[i] = &buf[i];
    }
    put_month(pbuf, year, month, 1);
    for (int i = 0; i < 8; i++) {
        io_write(STDOUT, buf[i].buf, buf[i].len);
        io_putc(STDOUT, L'\n');
    }
}

static void print_year(int year) {
    WCHAR data[8][(CAL_WIDTH + CAL_PADDING) * 3];
    print_year_header(year);
    int month = 1;
    for (int i = 0; i < 4; i++) {
        struct Buffer buf[8];
        struct Buffer* pbuf[8];
        for (int j = 0; j < 8; j++) {
            buf[j].buf = (WCHAR*)&data[j];
            buf[j].len = 0;
            pbuf[j] = &buf[j];
        }

        for (int j = 0; j < 3; j++) {
            put_month(pbuf, year, month, 0);
            month++;
        }

        for (int j = 0; j < 8; j++) {
            io_write(STDOUT, buf[j].buf, buf[j].len);
            io_putc(STDOUT, L'\n');
        }
        io_putc(STDOUT, L'\n');
    }
}
