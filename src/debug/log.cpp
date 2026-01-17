#include "log.h"

void logPrintf(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args); 
    va_end(args);
}

void logSamplingPrintf(const char *format, ...) {
    unsigned long nowTime = millis();
    if (nowTime % 1000 == 0) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}