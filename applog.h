#ifndef BITCOIN_INCLUDE_APPLOG_H
#define BITCOIN_INCLUDE_APPLOG_H

enum ApplogLevel {
	APPLOG_DEBUG,
	APPLOG_INFO,
	APPLOG_NOTICE,
	APPLOG_WARNING,
	APPLOG_ERROR,
	APPLOG_FATAL,
	APPLOG_BUG
};

void applog(enum ApplogLevel level,
	const char *function_name, const char *format,
	...
);

#endif
