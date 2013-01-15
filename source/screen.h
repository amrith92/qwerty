#ifndef _SCREEN_H_INCLUDED
#define _SCREEN_H_INCLUDED

#include <stdint.h>
#include <stdio.h>

static const uint8_t PRE_EDITOR = 2;
static const uint8_t POST_EDITOR = 3;
static const unsigned char STOPGAP = 200;

/**
 *	Represents various screen-buffer states
 */
typedef enum _screen_state {
	SCR_NORMAL, SCR_SCROLL_TOP, SCR_SCROLL_BOTTOM, SCR_SCROLL_LEFT, SCR_SCROLL_RIGHT
} ScreenState;

/**
 *	Represents current position in the screen buffer
 */
typedef struct _screen_pos {
	uint16_t row;
	uint16_t col;
} ScreenPosition;

/**
 *	Screen mode decides the behaviour of the screen
 *	The following modes exist:
 *		1. SCREEN_NORMAL
 *		2. SCREEN_INTR - Special interrupt mode
 */
typedef enum _screen_mode {
	SCREEN_NORMAL, SCREEN_INTR
} ScreenMode;

/**
 *	TODO:
 *	This structure keeps track of the format options for each character
 *	in the buffer
 *	A maximum of three format options available for each character:
 *		1. Bold, Underlined, Blinking, Concealed, Reversed
 *		2. Foreground Colour
 *		3. Background Colour
 *	To make use of this, i'll need to implement a hash-map or map
 *	to keep track of position of the character in the buffer
 */
typedef struct _screen_char_format {
	const unsigned char *fmt[3];
} ScreenCharFormat;

/**
 *	Represents the current screen-buffer
 */
typedef struct _screen {
	unsigned char **buffer;
	ScreenPosition pos;
	uint16_t max_row;
	uint16_t max_col;
	ScreenMode mode;
} Screen;

/**
 *	Create a new screen buffer
 */
extern uint8_t screen_new(Screen *screen, const uint16_t rows, const uint16_t cols);

/**
 *	Initialize screen buffer
 */
extern void screen_init(Screen *screen, const unsigned char *filename);

/**
 *	Deletes screen buffer
 */
extern void screen_release(Screen *screen);

/**
 *	Update cursor
 */
extern void screen_update_cursor(Screen *screen);

/**
 *	Writes the character c into the current position of the screen buffer
 *	This acts based on the current input
 */
extern uint8_t screen_write(Screen *screen, const unsigned char c);

/**
 *	Write a character to the current position in the screen buffer
 */
extern ScreenState screen_putc(Screen *screen, const unsigned char c);

/**
 *	Write a character to the specified position in the screen buffer
 */
extern void screen_putc_here(Screen *screen, const unsigned char c, const uint16_t col);

/**
 *	Advance row. It returns the state of the screen. Supposing that
 *	it needs to scroll, the caller has to decide what to do next
 */
extern ScreenState screen_advance_row(Screen *screen);

/**
 *	Retreat row. The same logic applies to this as screen_advance_row
 */
extern ScreenState screen_retreat_row(Screen *screen);

/**
 *	Shifts buffer down by one from the current position
 */
extern ScreenState screen_shift_down(Screen *screen);

/**
 *	Shifts buffer up by one to the current position
 */
extern ScreenState screen_shift_up(Screen *screen, const char *lastline);

/**
 *	Reset col position to beginning of line
 */
extern void screen_reset_col(Screen *screen);

/**
 *	Set col position
 */
extern ScreenState screen_set_col(Screen *screen, const uint16_t pos);

/**
 *	Set row position
 */
extern ScreenState screen_set_row_pos(Screen *screen, const uint16_t pos);

/**
 *	Move column position right. The same logic applies to this as
 *	screen_advance_row
 */
extern ScreenState screen_move_col_right(Screen *screen);

/**
 *	Move column position left. The same logic applies to this as
 *	screen_advance_row
 */
extern ScreenState screen_move_col_left(Screen *screen);

/**
 *	Removes the character (if there exists one) from
 *	the current position of the screen buffer
 */
extern void screen_delete(Screen *screen);

/**
 *	Clear the current line buffer
 */
extern void screen_clear_line(Screen *screen);

/**
 *	Flush screen buffer to output device
 */
extern void screen_flush_out(Screen *screen);

/**
 *	Ask for user input in response to question posed
 */
extern unsigned char screen_ask(Screen *screen, const char *question);

/**
 *	Add the menu to the buffer
 */
extern void screen_add_menu(Screen *screen);

#endif /* _SCREEN_H_INCLUDED */
