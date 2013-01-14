#include "tty.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

void tty_info_get(TtyInfo *info)
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

	info->rows = w.ws_row;
	info->cols = w.ws_col;
}

uint8_t tty_line_buffer_new(TtyLineBuffer *line)
{
	line->buffer = (unsigned char *) malloc(sizeof(unsigned char) * BUFSIZE);

	if (line->buffer == NULL) {
		return 3;
	}

	line->insertionPoint = 0;
	line->length = 0;

	return 0;
}

void tty_line_buffer_release(TtyLineBuffer *buf)
{
	if (buf->buffer != NULL) {
		free(buf->buffer);
	}
}

uint8_t tty_line_buffer_list_init(TtyLineBufferList *head)
{
	if (head == NULL) {
		head = (TtyLineBufferList *) malloc(sizeof(TtyLineBufferList));

		if (head == NULL)
			return 1;
	}

	if (tty_line_buffer_new(&head->line) > 0)
		return 2;

	head->prev = head->next = NULL;
	return 0;
}

uint16_t tty_line_buffer_list_length(TtyLineBufferList *head)
{
	TtyLineBufferList *cur = head;
	uint16_t ret = 0;

	while (cur != NULL) {
		++ret;
		cur = cur->next;
	}

	return ret;
}

void tty_line_buffer_list_grab_line_at(TtyLineBufferList *head, const uint16_t pos, const char **str)
{
	TtyLineBufferList *cur = head;
	uint16_t ret = 0;

	while (cur != NULL) {
		++ret;
		if (ret == pos) {
			*str = (const char *) cur->line.buffer;
			break;
		}
	}
}

uint8_t tty_line_buffer_list_new(TtyLineBufferList **cur)
{
	if ((*cur)->next == NULL) {
		(*cur)->next = (TtyLineBufferList *) malloc(sizeof(TtyLineBufferList));

		if ((*cur)->next == NULL)
			return 1;

		if (tty_line_buffer_new(&(*cur)->next->line)) {
			return 2;
		}

		(*cur)->next->prev = *cur;
		(*cur)->next->next = NULL;
		(*cur) = (*cur)->next;
	} else {
		/* If we need to insert a new line admidst others */
		TtyLineBufferList *tmp;
		tmp = (*cur)->next;
		(*cur)->next = (TtyLineBufferList *) malloc(sizeof(TtyLineBufferList));

		if ((*cur)->next == NULL) {
			return 3;
		}

		if (tty_line_buffer_new(&(*cur)->next->line)) {
			return 4;
		}

		(*cur)->next->next = tmp;
		(*cur)->next->prev = *cur;
		(*cur) = (*cur)->next;
		tmp->prev = *cur;
	}

	return 0;
}

uint8_t tty_line_buffer_list_del(TtyLineBufferList **cur)
{
	TtyLineBufferList *tmp;

	if (cur == NULL)
		return 1;

	tmp = *cur;
	if ((*cur)->prev != NULL)
		(*cur)->prev->next = (*cur)->next;
	if ((*cur)->next != NULL)
		(*cur)->next->prev = (*cur)->prev;

	tty_line_buffer_release(&(*cur)->line);

	free(tmp);
	return 0;
}

void tty_line_buffer_list_release(TtyLineBufferList *head)
{
	TtyLineBufferList *cur = head, *tmp;

	while (cur != NULL) {
		tmp = cur;
		cur = cur->next;
		tty_line_buffer_release(&tmp->line);
		free(tmp);
	}
}
