#ifndef _GPIO_H_
#define _GPIO_H_

#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>

#define MAX_BUF 512

#define ST7036_GPIO_DIR_IN  0
#define ST7036_GPIO_DIR_OUT 1

#define ST7036_GPIO_LOW     0
#define ST7036_GPIO_HIGH    1

int gpio_reserve(uint32_t gpio);
int gpio_set_direction(uint32_t gpio, int dir);
int gpio_set_value(uint32_t gpio, int val);

#endif
