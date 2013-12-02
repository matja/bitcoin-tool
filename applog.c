#define _POSIX_SOURCE

#include "applog.h"

#include <time.h>
#include <stdarg.h>
#include <stdio.h>

void applog(enum ApplogLevel level, const char *function_name, const char *format, ...)
{
	va_list args;
	FILE *file = stderr;
	char time_string[32] = { 0 };
	time_t time_now;
	struct tm tm_localtime;

	va_start(args, format);

/*	
	time_now = time(NULL);
	localtime_r(&time_now, &tm_localtime);
	strftime(time_string, sizeof(time_string)-1, "%Y-%m-%dT%H:%M:%S", &tm_localtime);
	fprintf(file, "%s|%s|", time_string, function_name);
	vfprintf(file, format, args);
	fprintf(file, "\n");
*/	

	vfprintf(file, format, args);
	fprintf(file, "\n");

	va_end(args);
}
