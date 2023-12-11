#ifndef _SNPRINTF_SS_H_INC_
#define _SNPRINTF_SS_H_INC_

#include <cstdarg>
#include <cstddef>
#include <cstdint>

/**
  Async-signal-safe utility functions used by signal handler routines.
  Declared here in order to unit-test them.
  These are not general-purpose, but tailored to the signal handling routines.
*/
/**
  Converts a int64_t value to string.
  @param   base 10 for decimal, 16 for hex values (0..9a..f)
  @param   val  The value to convert
  @param   buf  Assumed to point to the *end* of the buffer.
  @returns Pointer to the first character of the converted string.
           Negative values:
           for base-10 the return string will be prepended with '-'
           for base-16 the return string will contain 16 characters
  Implemented with simplicity, and async-signal-safety in mind.
*/
char* safe_itoa(int base, int64_t val, char* buf);

/**
  Converts a uint64_t value to string.
  @param   base 10 for decimal, 16 for hex values (0..9a..f)
  @param   val  The value to convert
  @param   buf  Assumed to point to the *end* of the buffer.
  @returns Pointer to the first character of the converted string.
  Implemented with simplicity, and async-signal-safety in mind.
*/
char* safe_utoa(int base, uint64_t val, char* buf);

/**
  A (very) limited version of snprintf.
  @param   to   Destination buffer.
  @param   n    Size of destination buffer.
  @param   fmt  printf() style format string.
  @returns Number of bytes written, including terminating '\0'
  Supports 'd' 'i' 'u' 'x' 'p' 's' conversion.
  Supports 'l' and 'll' modifiers for integral types.
  Does not support any width/precision.
  Implemented with simplicity, and async-signal-safety in mind.
*/
size_t safe_snprintf(char* to, size_t n, const char* fmt, ...)
    __attribute__((format(printf, 3, 4)));

/**
 @see `safe_snprintf` and `vsnprintf` */
size_t safe_vsnprintf(char* to, size_t size, const char* format, va_list ap);

#endif
