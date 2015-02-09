#include <types.h>
#include <string.h>

char *strcpy(char *dest, const char *src)
{
	size_t i = 0;

	while (src[i] != '\0') {
		dest[i] = src[i];
		i++;
	}

	dest[i] = '\0';

	return dest;
}

unsigned long strtoul(const char *nptr, const char **endptr, int base)
{
	unsigned long num = 0;

	while (isdigit_base(*nptr, base)) {
		unsigned long value;

		if (base == 16) {
			if (*nptr >= 'a' && *nptr <= 'f')
				value = *nptr - 'a' + 10;

			if (*nptr >= 'A' && *nptr <= 'F')
				value = *nptr - 'A' + 10;

			if (*nptr >= '0' && *nptr <= '9')
				value = *nptr - '0';
		}

		if (base == 10 || base == 8 || base == 2)
			value = *nptr - '0';

		num = num * base + value;

		nptr++;
	}

	if (endptr)
		*endptr = nptr;

	return num;
}

void *memcpy(void *dest, const void *src, size_t n)
{
	const uint8_t *s = src;
	uint8_t *d = dest;
	size_t i;

	for (i = 0; i < n; i++)
		d[i] = s[i];

	return dest;
}

void *memset(void *s, int c, size_t n)
{
	uint8_t *p = s;
	size_t i;

	for (i = 0; i < n; i++)
		p[i] = c;

	return s;
}
