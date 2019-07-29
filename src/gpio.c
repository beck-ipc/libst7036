#include "gpio.h"

int gpio_reserve(uint32_t gpio)
{
	int ret = 0;

	int fd;
	char buf[MAX_BUF];

	sprintf(buf, "%d", gpio);

	fd = open("/sys/class/gpio/export", O_WRONLY);

	if ((ret = fd) < 0)
	{
		close(fd);
		fprintf(stderr, "[ST7036] Error while trying to export gpio '%d': %s\n", gpio, strerror(errno));
		return ret;
	}

	write(fd, buf, strlen(buf));

	close(fd);

	return ret;
}

int gpio_set_direction(uint32_t gpio, int dir)
{
	int ret = 0;

	int fd;
	char dir_file[MAX_BUF];

	sprintf(dir_file, "/sys/class/gpio/gpio%d/direction", gpio);

	fd = open(dir_file, O_WRONLY);

	if ((ret = fd) < 0)
	{
		close(fd);
		fprintf(stderr, "[ST7036] Error while trying to set directon of gpio '%d': %s\n", gpio, strerror(errno));
	}

	if (dir == ST7036_GPIO_DIR_OUT)
	{
		write(fd, "out", 3);
	}
	else if (dir == ST7036_GPIO_DIR_IN)
	{
		write(fd, "in", 2);
	}


	close(fd);

	return ret;
}

int gpio_set_value(uint32_t gpio, int val)
{
	int ret = 0;

	int fd;
	char val_file[MAX_BUF];
	char buf[MAX_BUF];

	sprintf(val_file, "/sys/class/gpio/gpio%d/value", gpio);
	sprintf(buf, "%d", val);

	fd = open(val_file, O_WRONLY);

	if ((ret = fd) < 0)
	{
		close(fd);
		fprintf(stderr, "[ST7036] Error while trying to set value of gpio '%d': %s\n", gpio, strerror(errno));
	}

	write(fd, buf, sizeof(buf));

	close(fd);

	return ret;
}
