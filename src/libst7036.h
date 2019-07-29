/**
 * @file libst7036.h
 * @brief The header file of libst7036
 *
 * This header contains all accessible function of the libst7036 library. It
 * must be included in order to work with the display using the function of
 * the library.
 *
 * @example example.c
 */

#ifndef _LIBST7036_H_
#define _LIBST7036_H_

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
#include <linux/spi/spidev.h>

#include "gpio.h"

#define ST7036_SPI_MODE  SPI_MODE_0
#define ST7036_SPI_BITS  8
#define ST7036_SPI_SPEED 1000000
#define ST7036_SPI_DELAY 500

#define ST7036_FIRST_LONG_SLEEP     410
#define ST7036_EXEPTIONAL_LONG_WAIT 210
#define ST7036_ADDITIONAL_SLEEP      10

#define ST7036_COMMAND 0
#define ST7036_DATA    1

#define ST7036_CMD_DISPLAY_MODE              0b00001000
#define ST7036_CMD_POWER_ICON_CONSTRAST_HIGH 0b01010000
#define ST7036_CMD_CONTRAST_LOW              0b01110000
#define ST7036_CMD_FOLLOWER_CONTROL          0b01100000
#define ST7036_CMD_FUNCTION_SET              0b00111000
#define ST7036_CMD_ENTRY_MODE                0b00000100
#define ST7036_CMD_DDRAM_ADDRESS             0b10000000
#define ST7036_CMD_HOME                      0b00000010
#define ST7036_CMD_CLEAR                     0b00000001
#define ST7036_CMD_DOUBLE                    0b00010000
#define ST7036_CMD_BIAS                      0b00010100

#define ST7036_STAT_DISPLAY_ON               0b00000100
#define ST7036_STAT_CURSOR_VISIBLE           0b00000010
#define ST7036_STAT_CURSOR_BLINKING          0b00000001

#define ST7036_STAT_ICON_DISPLAY_ON          0b00001000
#define ST7036_STAT_SWITCH_BOOSTER_ENABLED   0b00000100
#define ST7036_STAT_GET_CONTRAST_HIGH(x)     ((x >> 4) & 0b00000011)
#define ST7036_STAT_GET_CONTRAST_LOW(x)      (x & 0b00001111)

#define ST7036_STAT_FOLLOWER_CONTROL_ENABLED 0b00001000
#define ST7036_STAT_V0_AMPLIFIED_RATIO       0b00000101

#define ST7036_STAT_DOUBLE_HEIGHT_ENABLED    0b00000100
#define ST7036_STAT_GET_INSTRUCTION_TABLE(x) (x)

#define ST7036_STAT_CURSOR_INCREMENT         0b00000010
#define ST7036_STAT_SHIFT_DISPLAY            0b00000001

#define ST7036_STAT_GET_DDRAM_ADDRESS(x)     (x)

#define ST7036_ROWS    3
#define ST7036_COLUMNS 16

/**
 *  @brief The context struct for the spi driven st7036 display
 *
 *  This C struct contains the current state of the st7036 display.
 *  You need to initialize it using st7036_init. It is not supposed
 *  to be manipulated manually, but only together with the st7036_*
 *  functions. Please note that this library only supports the 3
 *  line mode.
 */
typedef struct
{
	int fd;
	char *spi_dev;
	uint32_t mode;
	uint32_t bits;
	uint32_t speed;
	uint16_t delay;

	uint32_t rs_pin;
	uint32_t brt_pin;

	uint8_t double_height;
	uint8_t display_on;
	uint8_t cursor_visible;
	uint8_t cursor_blinking;

	uint8_t cursor_increment;
	uint8_t shift_display;
} st7036_t;

/**
 * @brief Initializes the display.
 *
 * This function does the proper initialization of the display
 * and sets some appropriate default value. These include a
 * visible and blinking cursor as well as a contrast of 20.
 *
 * @param st A pointer to an allocated st7036_t struct, which shall be used
 *           to handle the display.
 * @param spi_dev The spi device, that represents the connection to the
 *                display.
 * @param rs_pin The gpio number of the register select pin.
 * @param brt_pin The gpio number of the backlight pin.
 * @return Error code smaller than 0, if an error has occured, 0 otherwise.
 *
 * @see st7036_free
 */
int st7036_init(st7036_t *st, char *spi_dev, uint32_t rs_pin, uint32_t brt_pin);

/**
 * @brief Switches the backlight on or off.
 *
 * This function controls the backlight of the display and will switch it on
 * or off as desired.
 *
 * @param st The previously initialized st7036_t struct.
 * @param on The state of the backlight, TRUE or FALSE, TRUE for on and FALSE
 *           for off.
 * @return Error code smaller than 0, if an error has occured, 0 otherwise.
 */
int st7036_set_backlight(st7036_t *st, uint8_t on);

/**
 * @brief Sets the contrast.
 *
 * This function sets the contrast of the display. The contrast value must be
 * between 0 and 0x3F.
 *
 * @param st The previously initialized st7036_t struct.
 * @param contrast The contrast value to be set.
 * @return Error code smaller than 0, if an error has occured, 0 otherwise.
 */
int st7036_set_contrast(st7036_t *st, uint8_t contrast);

/**
 * @brief Sets the cursor position in two dimensions.
 *
 * This function sets the position of the cursor (x, y).
 *
 * @param st The previously initialized st7036_t struct.
 * @param x The x coordinate of the position.
 * @param y The y coordinate of the position.
 * @return Error code smaller than 0, if an error has occured, 0 otherwise.
 */
int st7036_set_cursor(st7036_t *st, uint8_t x, uint8_t y);

/**
 * @brief Sets wether the cursor bar should be visible or not.
 *
 * This function configures the visibility of the cursor bar, which indicates
 * the cursor position on the display.
 *
 * @param st The previously initialized st7036_t struct.
 * @param visible The visibility value, TRUE or FALSE.
 * @return Error code smaller than 0, if an error has occured, 0 otherwise.
 */
int st7036_set_cursor_visible(st7036_t *st, uint8_t visible);

/**
 * @brief Sets wether the cursor should be blinking.
 *
 * This function configures wether the cursor blinks or not.
 *
 * @param st The previously initialized st7036_t struct.
 * @param blinking The blinking value, TRUE or FALSE.
 * @return Error code smaller than 0, if an error has occured, 0 otherwise.
 */
int st7036_set_cursor_blinking(st7036_t *st, uint8_t blinking);

/**
 * @brief Clears the display.
 *
 * This function clears the display, so every character that was on it before
 * will be removed.
 *
 * @param st The previously initialized st7036_t struct.
 * @return Error code smaller than 0, if an error has occured, 0 otherwise.
 *
 * @see st7036_write_string
 * @see st7036_write_char
 */
int st7036_clear(st7036_t *st);

/**
 * @brief Displays a single character on the display.
 *
 * This function displays a signle character on the display at the current
 * position of the cursor.
 *
 * @param st The previously initialized st7036_t struct.
 * @param str The string, that will be displayed.
 * @return Error code smaller than 0, if an error has occured, 0 otherwise.
 *
 * @see st7036_set_cursor
 * @see st7036_write_string
 * @see st7036_clear
 */
int st7036_write_char(st7036_t *st, char c);

/**
 * @brief Displays a string on the display.
 *
 * This function displays every character of the given string on the display
 * in the given order and at the current position of the cursor. If a row is
 * full, the following characters will be continued on the next row.
 *
 * @param st The previously initialized st7036_t struct.
 * @param str The string, that will be displayed.
 * @return Error code smaller than 0, if an error has occured, 0 otherwise.
 *
 * @see st7036_set_cursor
 * @see st7036_write_char
 * @see st7036_clear
 */
int st7036_write_string(st7036_t *st, const char *str);

/**
 * @brief Frees the st7036_t struct.
 *
 * This function frees the st7036_t struct and all components that have been
 * initialized before.
 *
 * @param st The previously initialized st7036_t struct.
 * @return Error code smaller than 0, if an error has occured, 0 otherwise.
 *
 * @see st7036_init
 */
int st7036_free(st7036_t *st);

#endif
