#ifndef _EDITOR_H_INCLUDED
#define _EDITOR_H_INCLUDED

#include "tty.h"
#include "screen.h"

#define BACKUP_TIMEOUT	5

/**
 *	Represents an editor-session
 */
typedef struct _editor {
	TtyInfo ttyInfo;
	const char *filename;
	char *tempname;
	FILE *target;
	FILE *backup;
	TtyLineBufferList *head;
	TtyLineBufferList *cur;
	Screen screen;
	uint8_t is_dirty;
} Editor;

/**
 *	keeps a global reference to current editor
 */
extern Editor *gEditor;

/**
 *  Callback to handle SIGINT
 */
extern void editor_handle_sigint(int signum);

/**
 *	Initialize editor
 */
extern uint8_t editor_init(Editor *editor, const char *tgt);

/**
 *	Cleanup editor session
 */
extern void editor_release(Editor *editor);

/**
 *	Load file contents if content already exists
 */
extern void editor_load_from_file(Editor *editor);

/**
 *	Editor's main loop
 */
extern void editor_loopy(Editor *editor);

/**
 *	Exit editor safely
 */
extern uint8_t editor_safe_exit(Editor *editor);

/**
 *	Respond to input
 */
extern void editor_input(Editor *editor, const unsigned char in);

/**
 *	Grab a character
 */
extern unsigned char editor_getch();

/**
 *	Flush lines to specified file
 */
extern void editor_flush(Editor *editor, const char *fname, FILE *sink);

/**
 *	Backup data every five seconds
 */
extern void editor_perform_backup(int signum);

#endif /* _EDITOR_H_INCLUDED */
