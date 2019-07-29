#include "libst7036.h"

int main(int argc, char *argv[])
{
	int ret = 0;

	if (argc != 2)
	{
		fprintf(stderr, "Plz, I only want exactly one argument and this is what I will display!\n");
		return -1;
	}

	st7036_t st;

	ret = st7036_init(&st, "/dev/spidev0.0", 129, 502);
	if (ret < 0)
	{
		fprintf(stderr, "Unable to initialize display!\n");
		return ret;
	}

	st7036_set_backlight(&st, 1);
	st7036_set_cursor_blinking(&st, 1);
	st7036_set_cursor_visible(&st, 1);

	st7036_clear(&st);
	st7036_set_cursor(&st, 0, 0);
	st7036_write_string(&st, argv[1]);

	st7036_free(&st);

	return 0;
}
