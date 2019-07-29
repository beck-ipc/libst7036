#include "libst7036.h"

/* TODO: should be made private again */
int st7036_transfer(st7036_t *st, uint8_t const *tx, uint8_t *rx, size_t len, int is_cmd);
int st7036_transfer_instruction_set(st7036_t *st, uint8_t is);
int st7036_transfer_command(st7036_t *st, uint8_t val, uint8_t is);
int st7036_set_bias(st7036_t *st, uint8_t bias);
int st7036_send_follower_control(st7036_t *st);
int st7036_send_display_mode(st7036_t *st);
int st7036_send_entry_mode(st7036_t *st);

int st7036_init(st7036_t *st, char *spi_dev, uint32_t rs_pin, uint32_t brt_pin)
{
	int ret = 0;

	st->spi_dev = strdup(spi_dev);
	st->mode = ST7036_SPI_MODE;
	st->bits = ST7036_SPI_BITS;
	st->speed = ST7036_SPI_SPEED;
	st->delay = ST7036_SPI_DELAY;

	st->rs_pin = rs_pin;
	st->brt_pin = brt_pin;

	st->double_height = 0;
	st->display_on = 1;
	st->cursor_visible = 1;
	st->cursor_blinking = 1;

	st->cursor_increment = 1;
	st->shift_display = 0;

	st->fd = open(st->spi_dev, O_RDWR);
	if ((ret = st->fd) < 0)
	{
		close(st->fd);
		st->fd = 0;
		fprintf(stderr, "[ST7036] Error while opening '%s': %s\n", st->spi_dev, strerror(errno));
		return ret;
	}

	ret = ioctl(st->fd, SPI_IOC_WR_MODE32, &st->mode);
	if (ret < 0)
	{
		close(st->fd);
		st->fd = 0;
		fprintf(stderr, "[ST7036] Could not set SPI mode: %s\n", strerror(errno));
		return ret;
	}

	ret = ioctl(st->fd, SPI_IOC_WR_BITS_PER_WORD, &st->bits);
	if (ret < 0)
	{
		close(st->fd);
		st->fd = 0;
		fprintf(stderr, "[ST7036] Could not set SPI bits per word: %s\n", strerror(errno));
		return ret;
	}

	ret = ioctl(st->fd, SPI_IOC_WR_MAX_SPEED_HZ, &st->speed);
	if (ret < 0)
	{
		close(st->fd);
		st->fd = 0;
		fprintf(stderr, "[ST7036] Could not set SPI maximum speed: %s\n", strerror(errno));
		return ret;
	}

	ret = gpio_reserve(st->rs_pin);
	if (ret < 0)
	{
		close(st->fd);
		st->fd = 0;
		fprintf(stderr, "[ST7036] Could not reserve RS gpio pin: %s\n", strerror(errno));
		return ret;
	}

	ret = gpio_set_direction(st->rs_pin, ST7036_GPIO_DIR_OUT);
	if (ret < 0)
	{
		close(st->fd);
		st->fd = 0;
		fprintf(stderr, "[ST7036] Could not set direction of RS gpio pin: %s\n", strerror(errno));
		return ret;
	}

	ret = gpio_set_value(st->rs_pin, ST7036_GPIO_HIGH);
	if (ret < 0)
	{
		close(st->fd);
		st->fd = 0;
		fprintf(stderr, "[ST7036] Could not set value of RS gpio pin to high: %s\n", strerror(errno));
		return ret;
	}

	ret = gpio_reserve(st->brt_pin);
	if (ret < 0)
	{
		st->brt_pin = 0;
		fprintf(stderr, "[ST7036] Could not reserve BRT gpio pin: %s\n", strerror(errno));
		/* continue, not critical for display itself */
	}
	else
	{
		ret = gpio_set_direction(st->brt_pin, ST7036_GPIO_DIR_OUT);
		if (ret < 0)
		{
			st->brt_pin = 0;
			fprintf(stderr, "[ST7036] Could not set direction of BRT gpio pin: %s\n", strerror(errno));
			/* continue, not critical for display itself */
		}
	}

	st7036_transfer_instruction_set(st, 1);
	st7036_transfer_instruction_set(st, 1);
	st7036_set_bias(st, 1);
	st7036_set_contrast(st, 20);
	st7036_send_follower_control(st);
	st7036_send_display_mode(st);

	st7036_clear(st);
	st7036_send_entry_mode(st);

	return ret;
}

int st7036_set_backlight(st7036_t *st, uint8_t on)
{
	if (st->brt_pin)
	{
		return gpio_set_value(st->brt_pin, on ? ST7036_GPIO_LOW : ST7036_GPIO_HIGH);
	}
	else
	{
		return -1;
	}
}

int st7036_transfer(st7036_t *st, uint8_t const *tx, uint8_t *rx, size_t len, int is_cmd)
{
	int ret = 0;

	struct spi_ioc_transfer t =
	{
		.tx_buf = (unsigned int) tx,
		.rx_buf = (unsigned int) rx,
		.len = len,
		.delay_usecs = st->delay,
		.speed_hz = st->speed,
		.bits_per_word = st->bits,
	};

	ret = gpio_set_value(st->rs_pin, is_cmd == ST7036_COMMAND ? ST7036_GPIO_LOW : ST7036_GPIO_HIGH);
	if (ret < 0)
	{
		return ret;
	}

	ret = ioctl(st->fd, SPI_IOC_MESSAGE(1), &t);
	if (ret < 0)
	{
		fprintf(stderr, "[ST7036] Could not send SPI message: %s\n", strerror(errno));
	}

#ifdef DEBUG
	fprintf(stdout, "[ST7036] DEBUG: Sending %x\n", tx[0]);
#endif

	usleep(30);

	return ret;
}

int st7036_transfer_instruction_set(st7036_t *st, uint8_t is)
{
	int ret = 0;

	uint8_t tx[] = { ST7036_CMD_FUNCTION_SET |
	                 ST7036_STAT_GET_INSTRUCTION_TABLE(is) |
	                 (st->double_height ? ST7036_STAT_DOUBLE_HEIGHT_ENABLED : 0) };
	uint8_t rx[sizeof(tx) / sizeof(uint8_t)] = { 0, };
	
	ret = st7036_transfer(st, tx, rx, sizeof(tx) / sizeof(uint8_t), ST7036_COMMAND);

	return ret;
}

int st7036_transfer_command(st7036_t *st, uint8_t val, uint8_t is)
{
	int ret = 0;

	uint8_t tx[] = { val };
	uint8_t rx[sizeof(tx) / sizeof(uint8_t)] = { 0, };

	ret = st7036_transfer_instruction_set(st, is);
	if (ret < 0)
	{
		return ret;
	}

	ret = st7036_transfer(st, tx, rx, sizeof(tx) / sizeof(uint8_t), ST7036_COMMAND);

	return ret;
}

int st7036_send_display_mode(st7036_t *st)
{
	int ret = 0;

	uint8_t val = 0;
	
	val |= ST7036_CMD_DISPLAY_MODE;
	val |= st->display_on ? ST7036_STAT_DISPLAY_ON : 0;
	val |= st->cursor_visible ? ST7036_STAT_CURSOR_VISIBLE : 0;
	val |= st->cursor_blinking ? ST7036_STAT_CURSOR_BLINKING : 0;

#ifdef DEBUG
	fprintf(stdout, "[ST7036] DEBUG: Sending display mode");
#endif

	ret = st7036_transfer_command(st, val, 0);

	return ret;
}

int st7036_send_entry_mode(st7036_t *st)
{
	int ret = 0;
	uint8_t val = 0;
	
	val |= ST7036_CMD_ENTRY_MODE;
	val |= st->cursor_increment ? ST7036_STAT_CURSOR_INCREMENT : 0;
	val |= st->shift_display ? ST7036_STAT_SHIFT_DISPLAY : 0;

#ifdef DEBUG
	fprintf(stdout, "[ST7036] DEBUG: Sending entry mode");
#endif

	ret = st7036_transfer_command(st, val, 0);

	return ret;
}

int st7036_set_bias(st7036_t *st, uint8_t bias)
{
#ifdef DEBUG
	fprintf(stdout, "[ST7036] DEBUG: Sending bias\n");
#endif

	return st7036_transfer_command(st, ST7036_CMD_BIAS | (bias << 4) | 1, 1);
}

int st7036_set_contrast(st7036_t *st, uint8_t contrast)
{
	int ret = 0;

	if (contrast >= 0x40)
	{
		return -1;
	}

#ifdef DEBUG
	fprintf(stdout, "[ST7036] DEBUG: Sending contrast\n");
#endif

	ret = st7036_transfer_command(st, ST7036_CMD_CONTRAST_LOW |
	                                  ST7036_STAT_GET_CONTRAST_LOW(contrast), 1);
	if (ret < 0)
	{
		return ret;
	}

	ret = st7036_transfer_command(st, ST7036_CMD_POWER_ICON_CONSTRAST_HIGH |
	                                  ST7036_STAT_SWITCH_BOOSTER_ENABLED |
	                                  ST7036_STAT_GET_CONTRAST_HIGH(contrast), 1);
	return ret;
}

int st7036_send_follower_control(st7036_t *st)
{
	int ret = 0;

#ifdef DEBUG
	fprintf(stdout, "[ST7036] DEBUG: Sending follower control\n");
#endif

	ret = st7036_transfer_command(st, ST7036_CMD_FOLLOWER_CONTROL |
	                                  ST7036_STAT_FOLLOWER_CONTROL_ENABLED |
	                                  ST7036_STAT_V0_AMPLIFIED_RATIO, 1);

	return ret;
}

int st7036_set_cursor(st7036_t *st, uint8_t x, uint8_t y)
{
#ifdef DEBUG
	fprintf(stdout, "[ST7036] DEBUG: Sending cursor position\n");
#endif

	return st7036_transfer_command(st, ST7036_CMD_DDRAM_ADDRESS |
	                                   ST7036_STAT_GET_DDRAM_ADDRESS(y * ST7036_COLUMNS + x), 1);
}

int st7036_set_cursor_visible(st7036_t *st, uint8_t visible)
{
	st->cursor_visible = visible;

	return st7036_send_display_mode(st);
}

int st7036_set_cursor_blinking(st7036_t *st, uint8_t blinking)
{
	st->cursor_blinking = blinking;

	return st7036_send_display_mode(st);
}

int st7036_clear(st7036_t *st)
{
	return st7036_transfer_command(st, ST7036_CMD_CLEAR, 0);
}

int st7036_write_char(st7036_t *st, char c)
{
	int ret = 0;

	uint8_t tx[] = { c };
	uint8_t rx[sizeof(tx) / sizeof(uint8_t)] = { 0, };

#ifdef DEBUG
	fprintf(stdout, "[ST7036] DEBUG: Sending character\n");
#endif

	ret = st7036_transfer(st, tx, rx, sizeof(tx) / sizeof(uint8_t), ST7036_DATA);

	return ret;
}

int st7036_write_string(st7036_t *st, const char *str)
{
	int ret = 0;

	int i;
	for (i = 0; i < strlen(str); i++)
	{
		ret = st7036_write_char(st, str[i]);
		if (ret < 0)
		{
			return ret;
		}
	}

	return ret;
}

int st7036_free(st7036_t *st)
{
	int ret = 0;

	if (st->fd)
	{
		close(st->fd);
	}

	if (st->spi_dev)
	{
		free(st->spi_dev);
	}

	return ret;
}
