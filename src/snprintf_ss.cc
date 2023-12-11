#include "snprintf_ss.h"

static const char DIGITS[] = "0123456789abcdef";

char* safe_utoa(int base, uint64_t val, char* buf)
{
    *buf-- = 0;
    do {
        *buf-- = DIGITS[val % base];
    } while ((val /= base) != 0);
    return buf + 1;
}

char* safe_itoa(int base, int64_t val, char* buf)
{
    char* orig_buf    = buf;
    const bool is_neg = (val < 0);
    *buf--            = 0;

    if (is_neg)
        val = -val;
    if (is_neg && base == 16) {
        int ix;
        val -= 1;
        for (ix = 0; ix < 16; ++ix) {
            buf[-ix] = '0';
        }
    }

    do {
        *buf-- = DIGITS[val % base];
    } while ((val /= base) != 0);

    if (is_neg && base == 10) {
        *buf-- = '-';
    }

    if (is_neg && base == 16) {
        int ix;
        buf = orig_buf - 1;
        for (ix = 0; ix < 16; ++ix, --buf) {
            switch (*buf) {
            case '0':
                *buf = 'f';
                break;
            case '1':
                *buf = 'e';
                break;
            case '2':
                *buf = 'd';
                break;
            case '3':
                *buf = 'c';
                break;
            case '4':
                *buf = 'b';
                break;
            case '5':
                *buf = 'a';
                break;
            case '6':
                *buf = '9';
                break;
            case '7':
                *buf = '8';
                break;
            case '8':
                *buf = '7';
                break;
            case '9':
                *buf = '6';
                break;
            case 'a':
                *buf = '5';
                break;
            case 'b':
                *buf = '4';
                break;
            case 'c':
                *buf = '3';
                break;
            case 'd':
                *buf = '2';
                break;
            case 'e':
                *buf = '1';
                break;
            case 'f':
                *buf = '0';
                break;
            }
        }
    }
    return buf + 1;
}

static const char* check_longlong(const char* fmt, bool* have_longlong)
{
    *have_longlong = false;
    if (*fmt == 'l') {
        fmt++;
        if (*fmt != 'l') {
            *have_longlong = (sizeof(long) == sizeof(int64_t));
        } else {
            fmt++;
            *have_longlong = true;
        }
    }
    return fmt;
}

size_t safe_vsnprintf(char* to, size_t size, const char* format, va_list ap)
{
    char* start = to;
    char* end   = start + size - 1;
    for (; *format; ++format) {
        bool have_longlong = false;
        if (*format != '%') {
            if (to == end) {
                /* end of buffer */
                break;
            }
            *to++ = *format; /* copy ordinary char */
            continue;
        }
        ++format; /* skip '%' */

        format = check_longlong(format, &have_longlong);

        switch (*format) {
        case 'd':
        case 'i':
        case 'u':
        case 'x':
        case 'p': {
            int64_t ival  = 0;
            uint64_t uval = 0;
            if (*format == 'p') {
                have_longlong = (sizeof(void*) == sizeof(int64_t));
            }
            if (have_longlong) {
                if (*format == 'u') {
                    uval = va_arg(ap, uint64_t);
                } else {
                    ival = va_arg(ap, int64_t);
                }
            } else {
                if (*format == 'u') {
                    uval = va_arg(ap, unsigned int);
                } else {
                    ival = va_arg(ap, int);
                }
            }

            {
                char buff[22];
                const int base = (*format == 'x' || *format == 'p') ? 16 : 10;
                char* val_as_str =
                    (*format == 'u')
                        ? safe_utoa(base, uval, &buff[sizeof(buff) - 1])
                        : safe_itoa(base, ival, &buff[sizeof(buff) - 1]);

                /*
                  Strip off "ffffffff" if we have 'x' format without 'll'
                  Similarly for 'p' format on 32bit systems.
                */
                if (base == 16 && !have_longlong && ival < 0) {
                    val_as_str += 8;
                }

                while (*val_as_str && to < end) {
                    *to++ = *val_as_str++;
                }
                continue;
            }
        }
        case 's': {
            const char* val = va_arg(ap, char*);
            if (!val) {
                val = "(null)";
            }
            while (*val && to < end) {
                *to++ = *val++;
            }
            continue;
        }
        }
    }
    *to = 0;
    return to - start;
}

size_t safe_snprintf(char* to, size_t n, const char* fmt, ...)
{
    size_t result;
    va_list args;
    va_start(args, fmt);
    result = safe_vsnprintf(to, n, fmt, args);
    va_end(args);
    return result;
}
