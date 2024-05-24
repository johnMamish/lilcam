#include "cprintf.h"

#include <ctype.h>

void itoa(int n, char* out, int base)
{
    int i = 0;
    int is_negative = 0;
    char str[MAX_STRING_LENGTH];
    // Handle zero (0) as a special case
    if (n == 0)
    {
        out[0] = '0';
        out[1] = '\0';
	return;
    }

    // Check sign only if base 10.  Otherwise, cast directly to unsigned int.
    unsigned int num;
    if (n < 0 && base == 10)
    {
        is_negative = 1;
        n = -n;         // Make it positive now for calculations
	    num = n;
    }
    else
    {
	    num = (unsigned int)n;
    }

    // Determine conversion
    while (num != 0)
    {
        int remainder = num % base;

        // ASCII shift
        str[i++] = (remainder > 9) ? (remainder - 10) + 'a' : remainder + '0';

        num /= base;
    }

    if (is_negative)
    {
        str[i++] = '-';
    }

    for (int j = 0; j < i; j++)
    {
        out[j] = str[i - j - 1]; // Reverse the string
    }

    // Append string terminator
    str[i] = '\0';
}

void llutoa(uint64_t num, char* out)
{
    int BASE = 10;

    int i = 0;
    char str[MAX_STRING_LENGTH];
    // Handle zero (0) as a special case
    if (num == 0)
    {
        out[0] = '0';
        out[1] = '\0';
	return;
    }

    // Determine conversion
    while (num != 0)
    {
        uint64_t remainder = num % BASE;

        // ASCII shift
        str[i++] = (remainder > 9) ? (remainder - 10) + 'a' : remainder + '0';

        num /= BASE;
    }

    for (int j = 0; j < i; j++)
    {
        out[j] = str[i - j - 1]; // Reverse the string
    }

    // Append string terminator
    str[i] = '\0';
}

void lldtoa(int64_t n, char* out)
{
    int BASE = 10;

    int i = 0;
    int is_negative = 0;
    char str[MAX_STRING_LENGTH];
    // Handle zero (0) as a special case
    if (n == 0)
    {
        out[0] = '0';
        out[1] = '\0';
	return;
    }

    uint64_t num;
    if (n < 0)
    {
        is_negative = 1;
	    num = (uint64_t) (-n); // Make positive now for calculations
    }
    else
    {
	    num = (uint64_t) n;
    }

    // Determine conversion
    while (num != 0)
    {
        uint64_t remainder = num % BASE;

        // ASCII shift
        str[i++] = (remainder > 9) ? (remainder - 10) + 'a' : remainder + '0';

        num /= BASE;
    }

    if (is_negative)
    {
        str[i++] = '-';
    }

    for (int j = 0; j < i; j++)
    {
        out[j] = str[i - j - 1]; // Reverse the string
    }

    // Append string terminator
    str[i] = '\0';
}

int atoi(char* str)
{
    int len = strlen(str);
    int value = 0;

    int i;
    for (i = 0; i < len; i++)
    {
        value *= 10;
        value += str[i] - '0';
    }

    return value;
}

int is_digit(char c)
{
    return (c >= '0' && c <= '9');
}

void prepend(char* s, const char* t)
{
    int orig_len = strlen(s);
    int prep_len = strlen(t);

    // Make room for the prepended chars
    int i;
    for (i = orig_len - 1; i >= 0; i--)
    {
        s[i + prep_len] = s[i];
    }

    // Actually insert prepended chars
    for (i = 0; i < prep_len; i++)
    {
        s[i] = t[i];
    }
}

void prepend_n(char* s, const char t, int n)
{
    int width = n + 1;
    char ta[width];

    int i;
    for (i = 0; i < width - 1; i++)
    {
        ta[i] = t;
    }

    ta[i++] = '\0';

    prepend(s, ta);
}

void pad_string(char* str, int width, char pad)
{
    int len = strlen(str);
    int difference = width - len;

    if (difference > 0)
    {
        prepend_n(str, pad, difference);
    }

    return;
}

void put_string(void (*putc)(char), char* str)
{
    int i;
    for (i = 0; str[i] != '\0'; i++)
    {
        (*putc)(str[i]);
    }
}

void format_string(void (*putc)(char), va_list* args, int width)
{
    char* s = va_arg(*args, char*);
    pad_string(s, width, ' ');
    put_string(putc, s);
}

void format_integer(void (*putc)(char), va_list* args, int width)
{
    int value = va_arg(*args, int);
    char s[MAX_STRING_LENGTH] = "";
    itoa(value, s, 10);
    pad_string(s, width, '0');
    put_string(putc, s);
}

void format_llu(void (*putc)(char), va_list* args, int width)
{
    uint64_t value = va_arg(*args, uint64_t);
    char s[MAX_STRING_LENGTH] = "";
    llutoa(value, s);
    pad_string(s, width, '0');
    put_string(putc, s);
}

void format_lld(void (*putc)(char), va_list* args, int width)
{
    uint64_t value = va_arg(*args, int64_t);
    char s[MAX_STRING_LENGTH] = "";
    lldtoa(value, s);
    pad_string(s, width, '0');
    put_string(putc, s);
}

void format_hex(void (*putc)(char), va_list* args, int width)
{
    int value = va_arg(*args, int);
    char s[MAX_STRING_LENGTH] = "";
    itoa(value, s, 16);
    pad_string(s, width, '0');
    put_string(putc, s);
}

void cprintf(void (*putc)(char), const char* fmt, ...)
{
    va_list args;
    va_start(args, fmt);

    int i = 0;
    while (fmt[i] != '\0')
    {
        if (fmt[i] == '%')
        {
            i++;
            char c;
            switch (fmt[i])
            {
                case '%':       // Put a literal %
                    (*putc)('%');
                    break;
                case 'c':       // Character
                    c = va_arg(args, int);
                    (*putc)(c);
                    break;
                case 's':       // String
                    format_string(putc, &args, 0);
                    break;
                case 'i':       // Integer
                    format_integer(putc, &args, 0);
                    break;
                case 'l':       // Unsigned Long Long
                    if (fmt[i+1] == 'l' && fmt[i+2] == 'u')
                        format_llu(putc, &args, 0);
                    else if (fmt[i+1] == 'l' && fmt[i+2] == 'd')
                        format_lld(putc, &args, 0);
                    i += 2;
                    break;
                case 'p':       // pointer
                    format_hex(putc, &args, 0);
                    break;
                case 'x':       // Hexadecimal
                    format_hex(putc, &args, 0);
                    break;
                default:        // Handle everything else (width specifiers)
                    if (is_digit(fmt[i]))
                    {
                        char width_specifier[MAX_STRING_LENGTH] = "";
                        int k = 0;
                        while (is_digit(fmt[i]))
                        {
                            width_specifier[k++] = fmt[i++];
                        }
                        int width = atoi(width_specifier);

                        switch (fmt[i])
                        {
                            case 's':
                                format_string(putc, &args, width);
                                break;
                            case 'i':
                                format_integer(putc, &args, width);
                                break;
                            case 'l':
                                format_integer(putc, &args, width);
                                break;
                            case 'p':
                                format_hex(putc, &args, width);
                                break;
                            case 'x':
                                format_hex(putc, &args, width);
                                break;
                            default:
                                break; // XXX: We just silently ignore unrecognized flags
                        }
                    }
                    break;
            }
        }
        else
        {
            (*putc)(fmt[i]);
        }

        i++;
    }
    va_end(args);
    return;
}
