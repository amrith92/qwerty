#ifndef TTY_H_INCLUDED
#define TTY_H_INCLUDED

#include <stdint.h>

/**
 *	Default buffer size for each new line created
 */
static const uint64_t BUFSIZE = 512;

/**
 *	Stores info about the current terminal
 */
typedef struct _tty_info {
	uint16_t rows;
	uint16_t cols;
} TtyInfo;

/**
 *	Populates any TtyInfo object given to it
 */
extern void tty_info_get(TtyInfo *info);

/**
 *	Represents a line buffer
 */
typedef struct _tty_line_buffer {
	unsigned char *buffer;
	uint64_t insertionPoint;
	uint64_t length;
} TtyLineBuffer;

extern uint8_t tty_line_buffer_new(TtyLineBuffer *line);
extern void tty_line_buffer_release(TtyLineBuffer *buf);

/**
 *	Keeps a doubly linked list of variadic line-buffers
 */
typedef struct _tty_line_buffer_list {
	TtyLineBuffer line;
	struct _tty_line_buffer_list *next;
	struct _tty_line_buffer_list *prev;
} TtyLineBufferList;

/**
 *	Initializes a list
 */
extern uint8_t tty_line_buffer_list_init(TtyLineBufferList *head);

/**
 *	Get the number of lines
 */
extern uint16_t tty_line_buffer_list_length(TtyLineBufferList *head);

/**
 *	Get the line buffer at specified position in list
 */
extern void tty_line_buffer_list_grab_line_at(TtyLineBufferList *head, const uint16_t pos, const char **str);

/**
 *	Adds a new node to the buffer list and advances the current pointer
 */
extern uint8_t tty_line_buffer_list_new(TtyLineBufferList **cur);

/**
 *	Deletes the current line from the buffer list
 */
extern uint8_t tty_line_buffer_list_del(TtyLineBufferList **cur);

/**
 *	Destroys buffer list
 */
extern void tty_line_buffer_list_release(TtyLineBufferList *head);

#endif /* TTY_H_INCLUDED */
