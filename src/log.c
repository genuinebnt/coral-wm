#include <time.h>
#include <stdio.h>
#include <stdarg.h>

#include "log.h"

void Logger(const char* tag, const char* message, ...) {
    time_t now;
    time(&now);

    printf("[%.24s] %s: ", ctime(&now), tag);
    va_list args;
    va_start(args, message);
    printf(message, args);
    va_end(args);
}