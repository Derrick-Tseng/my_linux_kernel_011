#include <stdarg.h>
#include <string.h>

#define ZEROPAD 1       /* pad with zero */
#define SIGN    2       /* unsigned/signed long */
#define PLUS    4       /* show plus */
#define SPACE   8       /* space if plus */
#define LEFT    16      /* left justified */
#define SPECIAL 32      /* 0x */
#define SMALL   64      /* use 'abcdef' instead of 'ABCDEF' */

#define is_digit(c) ((c) >= '0' && (c) <= '9')

/**
 * Parses a sequence of decimal digits from the string pointed to by *s,
 * converts them to an integer value
 */
static int skip_atoi(const char **s) {
    int i=0;

    while (is_digit(**s)) {
        i = i*10 + *((*s)++) - '0';
    }

    return i;
}

/**
 * It divides n by base, stores the quotient back in n, and returns the remainder.
 */
#define do_div(n,base) ({ \
        int __res; \
        __asm__("divl %4":"=a" (n),"=d" (__res):"0" (n),"1" (0),"r" (base)); \
        __res; })

static char *number(char *str, int num, int base, int size, int precision, int type){
    char c, sign, tmp[36];
    const char *digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    int i = 0;

    if(type & SMALL)
        digits = "0123456789abcdefghijklmnopqrstuvwxyz";

    // If left justified, don't pad with zeros
    if(type & LEFT)
        type &= ~ZEROPAD; 

    if(base < 2 || base > 36)
        return 0;

    // Set Padding character    
    c = (type & ZEROPAD) ? '0' : ' '; 

    if((type & SIGN) && num < 0){
        sign = '-';
        num = -num; // Make num positive for processing
    } else {
        sign = (type & PLUS) ? '+' : ((type & SPACE) ? ' ' : 0);
    }

    // Reserve space for sign
    if(sign)
        size--;

    if(type & SPECIAL){
        if(base == 16)
            size -= 2; // Reserve space for "0x" or "0X"
        else if(base == 8)
            size--; // Reserve space for "0"
    }

    if(num == 0){
        tmp[i++] = '0';
    } else {
        while(num != 0){
            tmp[i++] = digits[do_div(num, base)];
        }
    }

    // Ensure precision is at least the number of digits
    if(i > precision)
        precision = i;
    // Adjust size for precision 
    size -= precision;

    // Pad with spaces or zeros
    if(!(type & (ZEROPAD + LEFT))){
        while(size-- > 0){
            *str++ = c;
        }
    }

    // Add sign if needed
    if(sign){
        *str++ = sign;
    }

    if(type & SPECIAL){
        if(base == 8){
            // Add '0' for octal
            *str++ = '0';
        }else if(base == 16){
            // Add '0x' or '0X for hexadecimal
            *str++ = '0';
            *str++ = (type & SMALL) ? 'x' : 'X';
        }
    }

    // Pad with spaces or zeros
    if(!(type & LEFT)){
        while(size-- > 0){
            *str++ = c;
        }
    }

    // Pad with zeros if precision is greater than number of digits
    while(i < precision--){
        *str++ = '0';
    }

    while(i-- > 0)
        *str++ = tmp[i];

    while(size-- > 0){
        *str++ = ' ';
    } 

    return str;
    
}

int vsprintf(char *buf, const char *fmt, va_list args){
    int len;
    int i;
    char *str;
    char *s;
    int *ip;

    int flags;
    int field_width;
    int precision;
    int qualifier;

    for(str = buf; *fmt; fmt++){
        if(*fmt != '%'){
            *str++ = *fmt;
            continue;
        }

        flags = 0;
        repeat:
            ++fmt; // Skip the '%'
            switch(*fmt){
                case '-': flags |= LEFT; goto repeat; // Left justify
                case '+': flags |= PLUS; goto repeat; // Force sign
                case ' ': flags |= SPACE; goto repeat; // Space if no sign
                case '#': flags |= SPECIAL; goto repeat; // Alternate form
                case '0': flags |= ZEROPAD; goto repeat; // Zero padding
            }

        field_width = -1;
        // e.g. %10d
        if(is_digit(*fmt)){
            field_width = skip_atoi(&fmt);
        }
        // e.g. %.*s
        // printf("%.*s", 3, "Testing"); -> "Tes"
        else if(*fmt == '*'){
            // Retrieve the next argument from the variadic argument list
            field_width = va_arg(args, int);
            // printf("%-10s", str) and printf("%*s", -10, str) produce identical left-justified output
            if(field_width < 0){
                field_width = -field_width;
                flags |= LEFT; // Left justify if negative width
            }
        }

        precision = -1;
        if(*fmt == '.'){
            ++fmt;
            if(is_digit(*fmt)){
                precision = skip_atoi(&fmt);
            }
            else if(*fmt == '*'){
                precision = va_arg(args, int);
            }

            if(precision < 0){
                precision = 0;
            }
        }

        qualifier = -1;
        if(*fmt == 'h' || *fmt == 'l' || *fmt == 'L'){
            qualifier = *fmt;
            ++fmt;
        }

        switch(*fmt){
            case('c'): {
                if(!(flags & LEFT)){
                    while(--field_width > 0){
                        *str++ = ' ';
                    }
                }
                *str++ = (unsigned char) va_arg(args, int); 
                while(--field_width > 0){
                    *str++ = ' ';
                }
                break;
            }
            case('s'): {
                s = va_arg(args, char *);
                len = strlen(s);

                if(precision < 0){
                    precision = len;
                }
                else if(len > precision){
                    len = precision;
                }

                if(!(flags & LEFT)){
                    while(--field_width > len){
                        *str++ = ' ';
                    }
                }
                for(i = 0; i < len; i++){
                    *str++ = *s++;
                }
                while(--field_width > len){
                    *str++ = ' ';
                }
                break;
            }
            case('o'): {
                str = number(str, va_arg(args, unsigned long), 8, field_width, precision, flags);
                break;
            }
            case('p'): {
                if (field_width == -1) {
                    field_width = 8;
                    flags |= ZEROPAD;
                }
                str = number(str, (unsigned long) va_arg(args, void *), 16, field_width, precision, flags);
                break;
            }
            case('x'): {
                flags |= SMALL; // Use lowercase for hex
            }
            case('X'): {
                str = number(str, va_arg(args, unsigned long), 16, field_width, precision, flags);
                break;
            }
            case('d'): {
            }
            case('i'): {
                flags = SIGN; // Default to signed integer
            }
            case('u'): {
                str = number(str, va_arg(args, unsigned long), 10, field_width, precision, flags);
                break;
            }
            case('n'): {
                ip = va_arg(args, int *);
                *ip = (str - buf);
                break;
            }
            default: {
                // Handle any character that follows a % but is not a recognized specifier
                if (*fmt != '%')
                    *str++ = '%';
               
                if (*fmt)
                    *str++ = *fmt;
                else
                    --fmt;
                break;
            }
        }
    }
    *str = '\0';
    return str-buf; // Return the length of the formatted string
}