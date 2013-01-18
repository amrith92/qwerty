#include "screen.h"
#include "colours.h"
#include "textproperties.h"
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

uint8_t screen_new(Screen *screen, const uint16_t rows, const uint16_t cols)
{
	size_t i;
	if (screen == NULL)
		return 1;

	screen->buffer = (unsigned char **) malloc(sizeof(unsigned char*) * (rows - 1));
	if (screen->buffer == NULL) {
		return 3;
	}

	for (i = 0; i < rows - 1; ++i) {
		screen->buffer[i] = (unsigned char *) malloc(sizeof(unsigned char) * cols);
		memset(screen->buffer[i], 0, cols);
	}

	screen->max_row = rows - 1;
	screen->max_col = cols;
	screen->pos.row = 0;
	screen->pos.col = 0;
	screen->mode = SCREEN_NORMAL;

	return 0;
}

void screen_init(Screen *screen, const unsigned char *filename)
{
	size_t i, len;
	for(i = 0; i < screen->max_col; ++i)
		screen->buffer[0][i] = '#';
	strcpy((char *)  screen->buffer[0] + 4, "[ qwerty 0.1 ]");
	screen->buffer[0][18] = '#';
	len = (int) (strlen((const char *) filename) / 2);
	screen->buffer[0][(int)(screen->max_col / 2) - len - 1] = ' ';
	screen->buffer[0][(int)(screen->max_col / 2) - len - 2] = '[';
	strcpy((char *) screen->buffer[0] + (int)(screen->max_col / 2) - len, (const char *) filename);
	screen->buffer[0][(int)(screen->max_col / 2) + len] = ' ';
	screen->buffer[0][(int)(screen->max_col / 2) + len + 1] = ']';
	screen->pos.row = 2;
	screen_add_menu(screen);
}

void screen_release(Screen *screen)
{
	size_t i;
	for (i = 0; i < screen->max_row; ++i)
		free(screen->buffer[i]);
	free(screen->buffer);
}

uint8_t screen_write(Screen *screen, const unsigned char c)
{
	uint8_t skip = 0;
	size_t idx = 0;

	if (screen->mode != SCREEN_NORMAL)
		return 1;

	switch (c) {
	case '\r':
	case '\n':
		screen->pos.row += (screen->pos.row < screen->max_row - POST_EDITOR) ? 1 : 0;
		screen->pos.col = 0;
		skip = 1;
	break;

	case '\t':
		screen->pos.col += 4;
		skip = 1;
	break;

	case '\b':
	case 127:
		--screen->pos.col;
		screen_delete(screen);
		skip = 1;
	break;

	case 178: /* HOME key */
		screen->pos.col = 0;
		skip = 1;
	break;

	case 176: /* END key */
		for (idx = 0; screen->buffer[screen->pos.row][idx] != '\0'; ++idx);
		screen->pos.col = idx;
		skip = 1;
	break;

	case 183: /* UP arrow key */
		screen->pos.row -= (screen->pos.row > PRE_EDITOR) ? 1 : 0;
		skip = 1;
	break;

	case 184: /* DOWN arrow key */
		screen->pos.row += (screen->pos.row < screen->max_row - POST_EDITOR) ? 1 : 0;
		if (screen->buffer[screen->pos.row][0] == '\0' && screen->buffer[screen->pos.row][1] == '\0')
			screen->pos.col = 0;
		skip = 1;
	break;

	case 186: /* LEFT arrow key */
		--screen->pos.col;
	 	skip = 1;
	break;

	case 185: /* RIGHT arrow key */
		screen->pos.col += (screen->pos.col + 1 < screen->max_col) ? 1 : 0;
		if (screen->buffer[screen->pos.row][screen->pos.col] == '\0')
			--screen->pos.col;
		skip = 1;
	break;
	}

	if (screen->pos.row < screen->max_row && screen->pos.col < screen->max_col && !skip)
		screen->buffer[screen->pos.row][screen->pos.col++] = c;
	else
		return 1;
	return 0;
}

ScreenState screen_putc(Screen *screen, const unsigned char c)
{
	size_t i;
	if (screen->pos.col >= screen->max_col) {
		/*	NOT IMPLEMENTED!
			The screen buffer isn't enough to place this character.
			We need to scroll right (FOR THIS LINE ALONE!)
		*/
	}
	if ('\t' == c) {
		for (i = 0; i < TAB_SIZE; ++i)
			screen->buffer[screen->pos.row][screen->pos.col++] = ' ';
		//screen->pos.col += TAB_SIZE;
	} else {
		screen->buffer[screen->pos.row][screen->pos.col++] = c;
	}
	return SCR_NORMAL;
}

void screen_putc_here(Screen *screen, const unsigned char c, const uint16_t col)
{
	screen->buffer[screen->pos.row][col] = c;
}

ScreenState screen_advance_row(Screen *screen)
{
	if (screen->pos.row >= screen->max_row - POST_EDITOR - 1)
		return SCR_SCROLL_BOTTOM;
	++screen->pos.row;
	return SCR_NORMAL;
}

ScreenState screen_retreat_row(Screen *screen)
{
	if (screen->pos.row <= PRE_EDITOR)
		return SCR_SCROLL_TOP;
	--screen->pos.row;
	return SCR_NORMAL;
}

ScreenState screen_shift_down(Screen *screen)
{
	size_t i;
	if (screen->pos.row >= screen->max_row - POST_EDITOR - 1) {
		/*
			We need to scroll the entire buffer
		*/
		return SCR_SCROLL_BOTTOM;
	}
	for (i = screen->max_row - POST_EDITOR - 2; i >= screen->pos.row; --i) {
		memcpy(screen->buffer[i + 1], screen->buffer[i], screen->max_col);
	}
	memset(screen->buffer[screen->pos.row], 0, screen->max_col);
	return SCR_NORMAL;
}

ScreenState screen_shift_up(Screen *screen, const char *lastline)
{
	size_t i;
	for (i = screen->pos.row; i < screen->max_row - POST_EDITOR - 1; ++i) {
		memcpy(screen->buffer[i], screen->buffer[i + 1], screen->max_col);
	}
	if (lastline != NULL) {
		memset(screen->buffer[screen->max_row - POST_EDITOR - 1], 0, screen->max_col);
		strcpy((char *) screen->buffer[screen->max_row - POST_EDITOR - 1], lastline);
	}
	return SCR_NORMAL;
}

void screen_reset_col(Screen *screen)
{
	screen->pos.col = 0;
}

ScreenState screen_set_col(Screen *screen, const uint16_t pos)
{
	if (pos > screen->max_col) {
		return SCR_SCROLL_RIGHT;
	}
	screen->pos.col = pos;
	return SCR_NORMAL;
}

ScreenState screen_insert_tab(Screen *screen)
{
	size_t i;
	if ((screen->pos.col + TAB_SIZE) > screen->max_col) {
		return SCR_SCROLL_RIGHT;
	}
	for (i = 0; i < TAB_SIZE; ++i)
		screen->buffer[screen->pos.row][screen->pos.col++] = ' ';
	//printf("\nSCREEN POS: %d\n", screen->pos.col);
	return SCR_NORMAL;
}

ScreenState screen_insert_tab_here(Screen *screen, const uint16_t col)
{
	size_t i;
	if (col > screen->max_col) {
		return SCR_SCROLL_RIGHT;
	}
	screen->pos.col = col;
	for (i = 0; i < TAB_SIZE; ++i)
		screen->buffer[screen->pos.row][screen->pos.col++] = ' ';
	return SCR_NORMAL;
}

ScreenState screen_set_row_pos(Screen *screen, const uint16_t pos)
{
	if (pos > screen->max_row)
		return SCR_SCROLL_BOTTOM;
	screen->pos.row = pos;
	return SCR_NORMAL;
}

ScreenState screen_move_col_right(Screen *screen)
{
	if (screen->pos.col >= screen->max_col)
		return SCR_SCROLL_RIGHT;
	++screen->pos.col;
	return SCR_NORMAL;
}

ScreenState screen_move_col_left(Screen *screen)
{
	if (screen->pos.col <= 0)
		return SCR_SCROLL_LEFT;
	--screen->pos.col;
	return SCR_NORMAL;
}

void screen_delete(Screen *screen)
{
	size_t i;
	for (i = screen->pos.col; i < screen->max_col - 1 && screen->buffer[screen->pos.row][i] != '\0'; ++i)
		screen->buffer[screen->pos.row][i] = screen->buffer[screen->pos.row][i + 1];
}

void screen_clear_line(Screen *screen)
{
	memset(screen->buffer[screen->pos.row], 0, screen->max_col);
}

void screen_update_cursor(Screen *screen)
{

}

void screen_flush_out(Screen *screen)
{
	size_t i, j;
	if (screen == NULL)
		return;

	for (i = 0; i < screen->max_row; ++i) {
		for (j = 0; j < screen->max_col && screen->buffer[i][j] != '\0'; ++j) {
			if (!i && j == 5) {
				putchar('\033');
				putchar('[');
				putchar('1');
				putchar('m');
			}
			if (!i && j == 16) {
				putchar('\033');
				putchar('[');
				putchar('0');
				putchar('m');
			}
			if ( i == screen->pos.row && j == ((screen->pos.col > 0) ? screen->pos.col - 1 : 0)) {
				putchar('\033');
				putchar('[');
				putchar('7');
				putchar('m');
				putchar(screen->buffer[i][j]);
				putchar('\033');
				putchar('[');
				putchar('0');
				putchar('m');
			} else
				putchar(screen->buffer[i][j]);
		}
		putchar('\n');
		fflush(stdout);
	}
}

unsigned char screen_ask(Screen *screen, const char *question)
{
	size_t i;
	screen->mode = SCREEN_INTR; /* Interrupt screen */
	for (i = 0; i < screen->max_col; ++i)
		screen->buffer[screen->max_row - POST_EDITOR][i] = '#';
	screen->buffer[screen->max_row - POST_EDITOR][1] = ' ';
	strcpy((char *) screen->buffer[screen->max_row - POST_EDITOR] + 2, question);
	screen->buffer[screen->max_row - POST_EDITOR][2 + strlen(question)] = ' ';
	memset(screen->buffer[screen->max_row - POST_EDITOR + 1], 0, screen->max_col);
	memset(screen->buffer[screen->max_row - POST_EDITOR + 2], 0, screen->max_col);
	strcpy((char *) screen->buffer[screen->max_row - POST_EDITOR + 1], "[y] Yes\t\t[n] No");
	strcpy((char *) screen->buffer[screen->max_row - POST_EDITOR + 2], "[c] Cancel");

	return 0;
}

void screen_add_menu(Screen *screen)
{
	size_t i;
	for (i = 0; i < screen->max_col; ++i)
		screen->buffer[screen->max_row - POST_EDITOR][i] = '#';
	memset(screen->buffer[screen->max_row - POST_EDITOR + 1], 0, screen->max_col);
	memset(screen->buffer[screen->max_row - POST_EDITOR + 2], 0, screen->max_col);
	strcpy((char *) screen->buffer[screen->max_row - POST_EDITOR + 1], "^O Write Out\t\t^V Cur Pos\t\t^K Cut Line");
	strcpy((char *) screen->buffer[screen->max_row - POST_EDITOR + 2], "^C Exit");
}
