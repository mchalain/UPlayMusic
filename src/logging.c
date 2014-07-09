#include <stdio.h>
#include <stdarg.h>

void Log_init(const char *filename) {

}

int Log_color_allowed(void) { return 0; }
#ifdef DEBUG
int Log_info_enabled(void) { return 1; }
#else
int Log_info_enabled(void) { return 0; }
#endif
int Log_error_enabled(void) { return 1; }

void Log_info(const char *category, const char *format, ...) {
	if (!Log_info_enabled()) return;
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
}

void Log_error(const char *category, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	vprintf(format, ap);
	va_end(ap);
}

