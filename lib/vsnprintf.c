#include <stdio.h>

uint32_t __div64_32(uint64_t *n, uint32_t base)
{
	uint64_t rem = *n;
	uint64_t b = base;
	uint64_t res, d = 1;
	uint32_t high = rem >> 32;

	/* Reduce the thing a bit first */
	res = 0;
	if (high >= base) {
		high /= base;
		res = (uint64_t)high << 32;
		rem -= (uint64_t)(high * base) << 32;
	}

	while ((int64_t)b > 0 && b < rem) {
		b = b + b;
		d = d + d;
	}

	do {
		if (rem >= b) {
			rem -= b;
			res += d;
		}

		b >>= 1;
		d >>= 1;
	} while (d);

	*n = res;
	return rem;
}

#define do_div(n, base) ({				\
	uint32_t __base = (base);			\
	uint32_t __rem;					\
	(void)(((typeof((n)) *)0) == ((uint64_t *)0));	\
	if (((n) >> 32) == 0) {				\
		__rem = (uint32_t)(n) % __base;		\
		(n) = (uint32_t)(n) / __base;		\
	} else {					\
		__rem = __div64_32(&(n), __base);	\
	}						\
	__rem;						\
})

#define LENGTH_NONE (0 << 0)
#define LENGTH_BYTE (1 << 0)
#define LENGTH_SHORT (2 << 0)
#define LENGTH_LONG (3 << 0)
#define LENGTH_LONG_LONG (4 << 0)
#define LENGTH_SIZE (5 << 0)

#define LENGTH_MASK (7 << 0)

#define SPECIAL (1 << 8)

static int number(char *str, char type, unsigned long flags, va_list ap)
{
	static const char digits[16] = "0123456789ABCDEF";
	unsigned long long value;
	unsigned int base = 10;
	char locase = 0x20;
	int count = 0, i;
	char tmp[20], ch;

	if (type == 'o')
		base = 8;

	if (type == 'x' || type == 'X') {
		if (flags & SPECIAL) {
			*str++ = '0';
			*str++ = type;
		}

		base = 16;
	}

	if (type == 'X')
		locase = 0;

	switch (flags & LENGTH_MASK) {
	case LENGTH_BYTE:
		value = (unsigned char)va_arg(ap, int);
		break;

	case LENGTH_SHORT:
		value = (unsigned short)va_arg(ap, int);
		break;

	case LENGTH_LONG:
		value = va_arg(ap, unsigned long);
		break;

	case LENGTH_LONG_LONG:
		value = va_arg(ap, unsigned long long);
		break;

	case LENGTH_SIZE:
		value = va_arg(ap, size_t);
		break;

	default:
		value = va_arg(ap, unsigned int);
		break;
	}

	if (value < base)
		str[count++] = digits[value];
	else {
		while (value)
			tmp[count++] = digits[do_div(value, base)] | locase;

		for (i = 0; i < count; i++)
			str[i] = tmp[count - i - 1];
	}

	if ((type == 'x' || type == 'X') && (flags & SPECIAL))
		count += 2;

	return count;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	const char *s;
	size_t i = 0;

	while (*format != '\0') {
		if (*format == '%') {
			unsigned long flags = 0;

			format++;

			if (*format == '#') {
				flags |= SPECIAL;
				format++;
			}

			if (*format == 'h') {
				format++;

				if (*format == 'h')
					flags |= LENGTH_BYTE;
				else
					flags |= LENGTH_SHORT;
			}

			if (*format == 'l') {
				format++;

				if (*format == 'l')
					flags |= LENGTH_LONG_LONG;
				else
					flags |= LENGTH_LONG;
			}

			if (*format == 'z') {
				format++;

				flags |= LENGTH_SIZE;
			}

			switch (*format) {
			case 'd':
			case 'i':
			case 'o':
			case 'u':
			case 'x':
			case 'X':
				i += number(str, *format, flags, ap);
				break;

			case 's':
				s = va_arg(ap, const char *);

				while (*s)
					str[i++] = *s++;

				break;

			default:
				str[i++] = *format;
				break;
			}
		} else {
			str[i++] = *format;
		}

		format++;
	}

	str[i++] = '\0';

	return 0;
}
