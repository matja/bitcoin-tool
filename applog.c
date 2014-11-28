#define _POSIX_SOURCE

#include "applog.h"
#include <time.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static struct tm *windows_localtime_r(const time_t *pt, struct tm *ptm)
{
	struct tm *tm = localtime(pt);
	memcpy(ptm, tm, sizeof(*ptm));
	return ptm;
}

static struct tm *portable_localtime_r(const time_t *pt, struct tm *ptm)
{
#if OS_FAMILY == Windows
	return windows_localtime_r(pt, ptm);
#else
	return localtime_r(pt, ptm);
#endif
}

void applog(enum ApplogLevel level, const char *function_name, const char *format, ...)
{
	va_list args;
	FILE *file = stderr;
	enum OutputFormat { NONE, STRING, TIME_FUNC_STRING } output_format = STRING;

	if (level < APPLOG_NOTICE) {
		/* users are not interested in debugging output. */
		/* TODO: make this dynamically configurable at run-time via an option */
		return;
	}

	va_start(args, format);

	switch (output_format) {
		case TIME_FUNC_STRING : {
			char time_string[32] = { 0 };
			time_t time_now;
			struct tm tm_localtime;

			time_now = time(NULL);
			portable_localtime_r(&time_now, &tm_localtime);
			strftime(time_string, sizeof(time_string)-1, "%Y-%m-%dT%H:%M:%S", &tm_localtime);
			fprintf(file, "%s|%s|", time_string, function_name);
			vfprintf(file, format, args);
			fprintf(file, "\n");
			break;
		}
		case STRING : {
			vfprintf(file, format, args);
			fprintf(file, "\n");
			break;
		}
		default :
			break;
	}

	va_end(args);
}
