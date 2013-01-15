#include "editor.h"
#include "textproperties.h"
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <termios.h>

Editor *gEditor = NULL;

volatile sig_atomic_t DO_AUTO_BACKUP = 0;

uint8_t editor_init(Editor *editor, const char *tgt)
{
	editor->tempname = (char *) malloc(sizeof(char) * (strlen(tgt) + 2));
	editor->filename = tgt;

	if (editor->tempname == NULL)
		return 1;

	strcpy(editor->tempname, tgt);
	strcat(editor->tempname, "~");

	editor->target = fopen(tgt, "a+");
	if (editor->target == NULL)
		return 2;
	fseek(editor->target, 0, SEEK_SET);

	editor->backup = fopen(editor->tempname, "w");
	if (editor->backup == NULL) {
		return 3;
	}

	editor->is_dirty = 0;
	editor->head = editor->cur = NULL;
	editor->head = (TtyLineBufferList *) malloc(sizeof(TtyLineBufferList));
	if (tty_line_buffer_list_init(editor->head))
		return 4;

	editor->cur = editor->head;

	// Set up screen buffer
	tty_info_get(&editor->ttyInfo);
	if (screen_new(&editor->screen, editor->ttyInfo.rows, editor->ttyInfo.cols)) {
		return 5;
	}

	screen_init(&editor->screen, (const unsigned char *) tgt);
	editor_load_from_file(editor);

	gEditor = editor;

	return 0;
}

void editor_release(Editor *editor)
{
	if (editor == NULL)
		return;
	if (editor->tempname != NULL)
		free(editor->tempname);
	if (editor->backup != NULL)
		fclose(editor->backup);
	if (editor->target != NULL)
		fclose(editor->target);
	tty_line_buffer_list_release(editor->head);
	screen_release(&editor->screen);
}

void editor_load_from_file(Editor *editor)
{
	int c = 0;
	if (editor == NULL)
		return;
	do {
		c = fgetc(editor->target);
		if (c != EOF)
			editor_input(editor, (unsigned char) c);
	} while (c != EOF);

	screen_reset_col(&editor->screen);
	screen_set_row_pos(&editor->screen, 0 + PRE_EDITOR);
	editor->cur = editor->head;
}

void editor_loopy(Editor *editor)
{
	unsigned char c = 0;
	signal(SIGINT, editor_handle_sigint);
	signal(SIGALRM, editor_perform_backup);
	alarm(BACKUP_TIMEOUT);
	while(1) {
		if (/*183 != c && 184 != c && 185 != c && 186 != c &&  178 != c && 176 != c && */ 169 != c)
			screen_flush_out(&editor->screen);
		c = editor_getch();
		/*editor->is_dirty = 1;
		screen_write(&editor->screen, c);*/
		editor_input(editor, c);
		usleep(200);
	}
}

void editor_input(Editor *editor, const unsigned char in)
{
	size_t i = 0;
	unsigned char tmp;
	const char *str;

	switch (in) {
	case '\n':
	case '\r':
		tty_line_buffer_list_new(&editor->cur);
		editor->is_dirty = 1;
		if (editor->cur->next == NULL) {
			screen_advance_row(&editor->screen);
			screen_reset_col(&editor->screen);
		} else {
			/*
				We have to shift the screen-buffer down by one
			*/
			screen_advance_row(&editor->screen);
			screen_shift_down(&editor->screen);
			screen_reset_col(&editor->screen);
		}
	break;

	case '\b':
	case 127:
		if (editor->cur->line.insertionPoint || editor->cur->line.length) {
			if (editor->cur->line.buffer[editor->cur->line.insertionPoint - 1] == '\t') {
				screen_move_col_left(&editor->screen);
				screen_delete(&editor->screen);
				screen_move_col_left(&editor->screen);
				screen_delete(&editor->screen);
				screen_move_col_left(&editor->screen);
				screen_delete(&editor->screen);
			}
			for (i = editor->cur->line.insertionPoint - 1; i < editor->cur->line.length; ++i)
				editor->cur->line.buffer[i] = editor->cur->line.buffer[i + 1];
			--editor->cur->line.insertionPoint;
			--editor->cur->line.length;
			editor->is_dirty = 1;
			screen_move_col_left(&editor->screen);
			screen_delete(&editor->screen);
		}
	break;

	case 11: /* Ctrl+K - Cut Line */
		if (editor->cur->prev == NULL) {
			/*
				If there's only one line, then simply empty the line's buffer
			*/
			memset(editor->cur->line.buffer, 0, BUFSIZE);
			screen_clear_line(&editor->screen);
			screen_reset_col(&editor->screen);
			if (editor->cur->next != NULL) {
				screen_shift_up(&editor->screen, NULL);
			}
			screen_retreat_row(&editor->screen);
		} else {
			if (!tty_line_buffer_list_del(&editor->cur)) {
				if (tty_line_buffer_list_length(editor->head) > editor->screen.max_row) {
					tty_line_buffer_list_grab_line_at(editor->head, editor->screen.max_row, &str);
					screen_shift_up(&editor->screen, str);
				} else {
					screen_shift_up(&editor->screen, NULL);
				}
			}
		}
	break;

	case 15: /* Ctrl+O - Save file */
		screen_ask(&editor->screen, "Save file?");
        screen_flush_out(&editor->screen);
        do {
            tmp = editor_getch();
            switch (tmp) {
            case 'y':
            case 'Y':
                editor_flush(editor, editor->filename, editor->target);
            case 'n':
            case 'N':
            case 'c':
            case 'C':
                tmp = 0;
            break;
            }
        } while (tmp != 0);
	break;

	case 169: /* DEL key */
		if (editor->cur->line.insertionPoint < editor->cur->line.length && editor->cur->line.length != 0) {
			for (i = editor->cur->line.insertionPoint; i < editor->cur->line.length - 1 && editor->cur->line.buffer[i] != '\0'; ++i)
                editor->cur->line.buffer[i] = editor->cur->line.buffer[i + 1];
			editor->is_dirty = 1;
			screen_delete(&editor->screen);
		}
	break;

	case 178: /* HOME key */
		editor->cur->line.insertionPoint = 0;
		screen_reset_col(&editor->screen);
	break;

	case 176: /* END key */
		editor->cur->line.insertionPoint = editor->cur->line.length;
		for (i = 0, tmp = 0; i < editor->cur->line.length; ++i)
			tmp += (editor->cur->line.buffer[i] == '\t') ? TAB_SIZE : 1;
		screen_set_col(&editor->screen, (unsigned int) tmp);
	break;

	case 183: /* UP arrow key */
		if (editor->cur->prev != NULL) {
			i = editor->cur->line.insertionPoint;
			editor->cur = editor->cur->prev;
			editor->cur->line.insertionPoint = (editor->cur->line.length <= i) ? editor->cur->line.length : i;
			screen_retreat_row(&editor->screen);
			for (i = 0, tmp = 0; i < editor->cur->line.insertionPoint; ++i)
				tmp += (editor->cur->line.buffer[i] == '\t') ? TAB_SIZE : 1;
			screen_set_col(&editor->screen, tmp);
		}
	break;

	case 184: /* DOWN arrow key */
		if (editor->cur->next != NULL) {
			i = editor->cur->line.insertionPoint;
			editor->cur = editor->cur->next;
			editor->cur->line.insertionPoint = (i >= editor->cur->line.length) ? editor->cur->line.length : i;
			screen_advance_row(&editor->screen);
			for (i = 0, tmp = 0; i < editor->cur->line.insertionPoint; ++i)
				tmp += (editor->cur->line.buffer[i] == '\t') ? TAB_SIZE : 1;
			screen_set_col(&editor->screen, tmp);
		}
	break;

	case 186: /* LEFT arrow key */
		editor->cur->line.insertionPoint -= (editor->cur->line.insertionPoint) ? 1 : 0;
		for (i = 0, tmp = 0; i < editor->cur->line.insertionPoint; ++i)
			tmp += (editor->cur->line.buffer[i] == '\t') ? TAB_SIZE : 1;
		screen_set_col(&editor->screen, tmp);
	break;

	case 185: /* RIGHT arrow key */
		editor->cur->line.insertionPoint += (editor->cur->line.insertionPoint < editor->cur->line.length) ? 1 : 0;
		for (i = 0, tmp = 0; i < editor->cur->line.insertionPoint; ++i)
			tmp += (editor->cur->line.buffer[i] == '\t') ? TAB_SIZE : 1;
		screen_set_col(&editor->screen, tmp);
	break;

	default:
		if (editor->cur->line.length <= editor->cur->line.insertionPoint)
			editor->cur->line.buffer[editor->cur->line.insertionPoint++] = in;
		else {
			for (i = editor->cur->line.length; i > editor->cur->line.insertionPoint; --i) {
				editor->cur->line.buffer[i] = editor->cur->line.buffer[i-1];
				screen_putc_here(&editor->screen, editor->cur->line.buffer[i], i);
			}
			editor->cur->line.buffer[editor->cur->line.insertionPoint++] = in;
		}
		screen_putc(&editor->screen, in);
		++editor->cur->line.length;
		editor->is_dirty = 1;
	}
	screen_add_menu(&editor->screen);
	if (DO_AUTO_BACKUP) {
		DO_AUTO_BACKUP = 0;
		editor_flush(editor, editor->tempname, editor->backup);
	}
}

uint8_t editor_safe_exit(Editor *editor)
{
	unsigned char in;
	if (editor == NULL)
		return 2;
	if (editor->is_dirty) {
		// Ask user if he/she really wants to quit
		screen_ask(&editor->screen, "Really quit?");
		screen_flush_out(&editor->screen);
		do {
			in = editor_getch();
			switch (in) {
			case 'y':
			case 'Y':
				editor_flush(editor, editor->filename, editor->target);
				return 1;

			case 'n':
			case 'N':
			case 'c':
			case 'C':
				return 0;
			}
		} while (in != 0);
	}

	return 1;
}

void editor_handle_sigint(int signum)
{
    signal(signum, SIG_IGN);
    if(editor_safe_exit(gEditor)) {
	   	editor_release(gEditor);
		gEditor = NULL;
		system("stty sane");
	    exit(0);
	} else {
		screen_add_menu(&gEditor->screen);
		screen_flush_out(&gEditor->screen);
		signal(signum, editor_handle_sigint);
	}
}

unsigned char editor_getch() {
	unsigned char buf = 0, arrow = 0;
	struct termios oldstuff;
	struct termios newstuff;

	tcgetattr(STDIN_FILENO, &oldstuff);
	newstuff = oldstuff;                  /* save old attributes               */
	newstuff.c_lflag &= ~(ICANON | ECHO); /* reset "canonical" and "echo" flags*/
	newstuff.c_cc[VMIN] = 1;
	newstuff.c_cc[VTIME] = 0;
	tcsetattr(STDIN_FILENO, TCSANOW, &newstuff); /* set new attributes         */
	buf = getchar();

	//printf("CHAR=[%d] ", buf);

	if (buf == 27) {
		arrow = buf & 0xFF;
		arrow += getchar() & 0xFF;
		//printf("ARROW.1[%d]", arrow);
		arrow += getchar() & 0xFF;
		//printf("ARROW.2[%d]", arrow);
		switch (arrow) {
		case 169:	/* DEL key */
		case 176:	/* END key */
		case 178:	/* HOME key */
		case 183:	/* UP arrow key */
		case 184:	/* DOWN arrow key */
		case 185:	/* RIGHT arrow key */
		case 186:	/* LEFT arrow key */
			break;
		default: arrow = 0;
		}
		if (0 != arrow)
			buf = arrow;
	}

	tcsetattr(STDIN_FILENO, TCSANOW, &oldstuff); /* restore old attributes     */

	if (buf == 0x04)
		buf = EOF;

	return (buf);
}

void editor_flush(Editor *editor, const char *fname, FILE *sink)
{
	TtyLineBufferList *cur = NULL;
	size_t i = 0;

	if (editor == NULL)
		return;
	if (sink == NULL)
		return;

	freopen(fname, "w", sink);

	cur = editor->head;
	while (cur != NULL) {
		for (i = 0; i < cur->line.length; ++i)
			fputc(cur->line.buffer[i], sink);
		cur = cur->next;
		if (cur != NULL)
			fputc('\n', sink);
	}
}

void editor_perform_backup(int signum)
{
	signal(SIGALRM, SIG_IGN);
	//editor_flush(gEditor, gEditor->tempname, gEditor->backup);
	DO_AUTO_BACKUP = 1;
	signal(SIGALRM, editor_perform_backup);
	alarm(BACKUP_TIMEOUT);
}
