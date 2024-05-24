/**
 * @author Sean Messenger
 *
 * @brief Contains compact printf function
 */

#ifndef CPRINTF_H
#define CPRINTF_H


#include <stdarg.h>
#include <string.h>
#include <stdint.h>

#define MAX_STRING_LENGTH 100

/**
 * @brief Acts as a compact and limited printf function
 * @details To be used for a serial interface with the
 * samd2x series microcontrollers. This limits the stack
 * space use and runs much faster than alternatives.
 *
 * putc needs to be a pointer to a function that
 * takes a char.
 *
 * The fmt string can be any char array and recognizes
 * commands of the following format:
 *      %[width]flag
 * where [width] is an optional integer minimum width.
 * The function pads out to that, filling with spaces for
 * strings or zeros for numbers. The width specifier
 * can be used for flags marked with [*] below.
 *
 * The supported flags are:
 *      % - Gives a literal % sign
 *      c - Formats a character
 *      s - Formats a string[*]
 *      i - Formats an integer[*]
 *      p - Formats an address[*]
 *      x - Formats hexadecimal[*]
 */
void cprintf(void (*putc)(char), const char* fmt, ...);

#endif
