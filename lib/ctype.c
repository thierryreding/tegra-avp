#include <ctype.h>

int isdigit(int c)
{
	return c >= '0' && c <= '9';
}

int isxdigit(int c)
{
	return (c >= '0' && c <= '9') ||
	       (c >= 'a' && c <= 'f') ||
	       (c >= 'A' && c <= 'F');
}

int isdigit_base(int c, int base)
{
	if (base == 2)
		return c >= '0' && c <= '1';

	if (base == 8)
		return c >= '0' && c <= '7';

	if (base == 10)
		return isdigit(c);

	if (base == 16)
		return isxdigit(c);

	return 0;
}

int isprint(int ch)
{
	/* 0x20 (space) - 0x7e (tilde) */
	return ch >= ' ' && ch <= '~';
}
