#include <stdio.h>
#include "editor.h"

int main(int argc, char **argv)
{
	if (argc < 2) {
		printf("\033[1mUsage:\033[0m \033[32mqwerty\033[0m filename\n");
		return 0;
	}

	Editor editor;
	if (editor_init(&editor, argv[1])) {
		printf("\nArgh. Something went wrong :(\n");
		return 0;
	}
	editor_loopy(&editor);

	return 0;
}
