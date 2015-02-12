#include <ctype.h>
#include <string.h>
#include <types.h>

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
		unsigned long value = 0;

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
	const void *s = src, *end = src + n;
	void *d = dest;

	while (s <= end - 4) {
		*(uint32_t *)d = *(uint32_t *)s;
		d += 4;
		s += 4;
	}

	if (s <= end - 2) {
		*(uint16_t *)d = *(uint16_t *)s;
		d += 2;
		s += 2;
	}

	if (s < end)
		*(uint8_t *)d = *(uint8_t *)s;

	return dest;
}

void *memset(void *s, int c, size_t n)
{
	void *ptr = s, *end = s + n;

	while (ptr <= end - 4) {
		uint32_t *p = ptr;
		*p = (c << 24) | (c << 16) | (c << 8) | c;
		ptr += 4;
	}

	if (ptr <= end - 2) {
		uint16_t *p = ptr;
		*p = (c << 8) | c;
		ptr += 2;
	}

	if (ptr < end) {
		uint8_t *p = ptr;
		*p = c;
	}

	return s;
}
