#ifndef DEBUG_H
#define DEBUG_H

#include "../defs.h"
#include <stdio.h>

/* Styles */
#define DBG_NO_COLOR ""
#define DBG_RESET "\033[0m"
#define DBG_BOLD "\033[1m"
#define DBG_DISABLE "\033[2m"
#define DBG_ITALIC "\033[3m"
#define DBG_UNDERLINE "\033[4m"
#define DBG_INVERT "\033[7m"
#define DBG_INVISIBLE "\033[8m"
#define DBG_STRIKETHROUGH "\033[9m"
#define DBG_UNDERLINE_2 "\033[21m"

/* Foreground colors */
#define DBG_BLACK "\033[30m"
#define DBG_DARK_RED "\033[31m"
#define DBG_DARK_GREEN "\033[32m"
#define DBG_DARK_YELLOW "\033[33m"
#define DBG_DARK_BLUE "\033[34m"
#define DBG_DARK_PURPLE "\033[35m"
#define DBG_DARK_TURQUOISE "\033[36m"
#define DBG_GRAY "\033[37m"

/* Background colors */
#define DBG_BG_BLACK "\033[40m"
#define DBG_BG_DARK_RED "\033[41m"
#define DBG_BG_DARK_GREEN "\033[42m"
#define DBG_BG_DARK_YELLOW "\033[43m"
#define DBG_BG_DARK_BLUE "\033[44m"
#define DBG_BG_DARK_PURPLE "\033[45m"
#define DBG_BG_DARK_TURQUOISE "\033[46m"
#define DBG_BG_GRAY "\033[47m"

/* Bright foreground colors */
#define DBG_DARK_GRAY "\033[90m"
#define DBG_RED "\033[91m"
#define DBG_GREEN "\033[92m"
#define DBG_YELLOW "\033[93m"
#define DBG_BLUE "\033[94m"
#define DBG_PURPLE "\033[95m"
#define DBG_TURQUOISE "\033[96m"
#define DBG_WHITE "\033[97m"

/* Bright background colors */
#define DBG_BG_DARK_GRAY "\033[100m"
#define DBG_BG_RED "\033[101m"
#define DBG_BG_GREEN "\033[102m"
#define DBG_BG_YELLOW "\033[103m"
#define DBG_BG_BLUE "\033[104m"
#define DBG_BG_PURPLE "\033[105m"
#define DBG_BG_TURQUOISE "\033[106m"
#define DBG_BG_WHITE "\033[107m"

/* Aliases */
#define DBG_FG_DEFAULT DBG_GRAY
#define DBG_BG_DEFAULT DBG_BG_BLACK
#define DBG_HEADER DBG_PURPLE
#define DBG_OKBLUE DBG_BLUE
#define DBG_OKCYAN DBG_TURQUOISE
#define DBG_OKGREEN DBG_GREEN
#define DBG_WARNING DBG_YELLOW
#define DBG_FAIL DBG_RED
#define DBG_ENDC DBG_RESET

void debug_log(const char* msg, const char* color, const char* end, FILE* stream, b8 flush);
void debug_logf(const char* color, FILE* stream, b8 flush, const char* fmt, ...);

#define DEBUG_LOG(msg) debug_log((msg), DBG_NO_COLOR, "\n", NULL, 0)
#define DEBUG_LOG_COLOR(msg, color) debug_log((msg), (color), "\n", NULL, 0)

#endif
