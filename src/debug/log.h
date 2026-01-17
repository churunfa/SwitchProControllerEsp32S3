#ifndef SWITCHPROCONTROLLERESP32S3_LOG_H
#define SWITCHPROCONTROLLERESP32S3_LOG_H

#include <Arduino.h>

void logPrintf(const char *format, ...);
void logSamplingPrintf(const char *format, ...);

#endif //SWITCHPROCONTROLLERESP32S3_LOG_H