#ifndef _COLOURS_H_INCLUDED
#define _COLOURS_H_INCLUDED

const unsigned char *BOLD		= (const unsigned char *) "\033[1m";
const unsigned char *UNDERLINED	= (const unsigned char *) "\033[4m";
const unsigned char *BLINKING	= (const unsigned char *) "\033[5m";
const unsigned char *REVERSED	= (const unsigned char *) "\033[7m";
const unsigned char *CONCEALED	= (const unsigned char *) "\033[8m";

const unsigned char *FG_BLACK	= (const unsigned char *) "\033[30m";
const unsigned char *FG_RED		= (const unsigned char *) "\033[31m";
const unsigned char *FG_GREEN	= (const unsigned char *) "\033[32m";
const unsigned char *FG_YELLOW	= (const unsigned char *) "\033[33m";
const unsigned char *FG_BLUE	= (const unsigned char *) "\033[34m";
const unsigned char *FG_MAGENTA	= (const unsigned char *) "\033[35m";
const unsigned char *FG_CYAN	= (const unsigned char *) "\033[36m";
const unsigned char *FG_WHITE	= (const unsigned char *) "\033[37m";

const unsigned char *BG_BLACK	= (const unsigned char *) "\033[40m";
const unsigned char *BG_RED		= (const unsigned char *) "\033[41m";
const unsigned char *BG_GREEN	= (const unsigned char *) "\033[42m";
const unsigned char *BG_YELLOW	= (const unsigned char *) "\033[43m";
const unsigned char *BG_BLUE	= (const unsigned char *) "\033[44m";
const unsigned char *BG_MAGENTA	= (const unsigned char *) "\033[45m";
const unsigned char *BG_CYAN	= (const unsigned char *) "\033[46m";
const unsigned char *BG_WHITE	= (const unsigned char *) "\033[47m";

const unsigned char *FGBG_RESET	= (const unsigned char *) "\033[0m";

#endif /* _COLOURS_H_INCLUDED */
