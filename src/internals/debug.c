#include "debug.h"
#include <stdarg.h>
#include <string.h>
#include <time.h>

static void debug_print_timestamp(FILE* stream)
{
    time_t now;
    struct tm local_tm;
    char time_buf[16];

    now = time(NULL);

#if defined(_WIN32)
    localtime_s(&local_tm, &now);
#else
    localtime_r(&now, &local_tm);
#endif

    if (strftime(time_buf, sizeof(time_buf), "%H:%M:%S", &local_tm) == 0) {
        memcpy(time_buf, "00:00:00", 9);
    }

    fprintf(stream, "%s[%s]%s  ", DBG_DARK_GRAY, time_buf, DBG_RESET);
}

void debug_log(const char* msg, const char* color, const char* end, FILE* stream, b8 flush)
{
    FILE* out;
    const char* paint;
    const char* trailer;

    out = (stream != NULL) ? stream : stdout;
    paint = (color != NULL) ? color : DBG_NO_COLOR;
    trailer = (end != NULL) ? end : "\n";

    debug_print_timestamp(out);

    if (paint[0] != '\0') {
        fprintf(out, "%s%s%s", paint, (msg != NULL) ? msg : "", DBG_RESET);
    } else {
        fputs((msg != NULL) ? msg : "", out);
    }

    fputs(trailer, out);

    if (flush) {
        fflush(out);
    }
}

void debug_logf(const char* color, FILE* stream, b8 flush, const char* fmt, ...)
{
    FILE* out;
    const char* paint;
    va_list args;

    out = (stream != NULL) ? stream : stdout;
    paint = (color != NULL) ? color : DBG_NO_COLOR;

    debug_print_timestamp(out);

    if (paint[0] != '\0') {
        fputs(paint, out);
    }

    va_start(args, fmt);
    vfprintf(out, (fmt != NULL) ? fmt : "", args);
    va_end(args);

    if (paint[0] != '\0') {
        fputs(DBG_RESET, out);
    }

    fputc('\n', out);

    if (flush) {
        fflush(out);
    }
}
