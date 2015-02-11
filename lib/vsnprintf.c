#include <ctype.h>
#include <stdio.h>
#include <string.h>

#define do_div(n, base) ({		\
	typeof(n) __rem = (n) % (base);	\
	(n) = (n) / (base);		\
	__rem;				\
})

enum {
	NONE,
	BYTE,
	SHORT,
	LONG,
	LONG_LONG,
	SIZE_T,
};

#define ALTERNATE (1 << 0)
#define LOWERCASE (1 << 1)
#define ZEROPAD   (1 << 2)

struct printf_spec {
	unsigned int base;
	unsigned long length;
	unsigned long flags;
	unsigned int width;
	char conv;
};

static const char *parse_spec(const char *format, struct printf_spec *spec)
{
	if (*format == '#') {
		spec->flags |= ALTERNATE;
		format++;
	}

	if (*format == '0') {
		spec->flags |= ZEROPAD;
		format++;
	}

	if (*format == '-') {
		/* pad with spaces */
		format++;
	}

	if (isdigit(*format)) {
		const char *end;

		spec->width = strtoul(format, &end, 10);

		format = end;
	}

	switch (*format) {
	case 'h':
		if (format[1] == 'h') {
			spec->length = BYTE;
			format++;
		} else {
			spec->length = SHORT;
		}

		format++;
		break;

	case 'l':
		if (format[1] == 'l') {
			spec->length = LONG_LONG;
			format++;
		} else {
			spec->length = LONG;
		}

		format++;
		break;

	case 'z':
		spec->length = SIZE_T;
		format++;
		break;
	}

	spec->conv = *format++;

	switch (spec->conv) {
	case 'o':
		spec->base = 8;
		break;

	case 'p':
		spec->flags |= ZEROPAD | LOWERCASE | ALTERNATE;
		spec->length = LONG;
		spec->width = 8;
		spec->base = 16;
		break;

	case 'x':
		spec->flags |= LOWERCASE;
	case 'X':
		spec->base = 16;
		break;

	case 'd':
	case 'i':
	case 'u':
		spec->base = 10;
		break;
	}

	return format;
}

static char *number(char *str, struct printf_spec *spec, unsigned long value)
{
	char locase = spec->flags & LOWERCASE ? 0x20 : 0x00;
	static const char digits[16] = "0123456789ABCDEF";
	unsigned int count = 0, i, width = spec->width;
	char tmp[22];

	do {
		unsigned char index = do_div(value, spec->base);

		tmp[count++] = digits[index] | locase;
	} while (value > 0);

	/* add prefix */
	if (spec->base == 16 && spec->flags & ALTERNATE) {
		*str++ = '0';

		if (spec->flags & LOWERCASE)
			*str++ = 'x';
		else
			*str++ = 'X';

		/* adjust field width to account for the prefix */
		if (width > 1)
			width -= 2;
	}

	if (count < width) {
		for (i = 0; i < width - count; i++) {
			if (spec->flags & ZEROPAD)
				*str++ = '0';
			else
				*str++ = ' ';
		}
	}

	for (i = 0; i < count; i++)
		*str++ = tmp[count - i - 1];

	return str;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
	char *end = str + size;
	char *start = str;
	const char *s;

	while (str < end && *format != '\0') {
		if (*format == '%') {
			struct printf_spec spec = { 0, };
			unsigned long value;

			format = parse_spec(format + 1, &spec);

			switch (spec.conv) {
			case 'd':
			case 'i':
			case 'o':
			case 'u':
			case 'x':
			case 'X':
			case 'p':
				switch (spec.length) {
				case BYTE:
					value = (unsigned char)va_arg(ap, int);
					break;

				case SHORT:
					value = (unsigned short)va_arg(ap, int);
					break;

				case LONG:
					value = va_arg(ap, unsigned long);
					break;

				/*
				case LONG_LONG:
					value = va_arg(ap, unsigned long long);
					break;
				*/

				case SIZE_T:
					value = va_arg(ap, size_t);
					break;

				default:
					value = va_arg(ap, unsigned int);
					break;
				}

				str = number(str, &spec, value);
				break;

			case 's':
				s = va_arg(ap, const char *);

				while (str < end && *s != '\0')
					*str++ = *s++;

				break;

			default:
				*str++ = spec.conv;
				break;
			}
		} else {
			*str++ = *format++;
		}
	}

	*str++ = '\0';

	return str - start;
}
