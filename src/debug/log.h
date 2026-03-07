#ifndef SWITCHPROCONTROLLERESP32S3_LOG_H
#define SWITCHPROCONTROLLERESP32S3_LOG_H

#include <Arduino.h>
#include <string>

void logPrintf(const char *format, ...);
void logSamplingPrintf(const char *format, ...);
inline std::string string_printf(const char* format, ...) {
    va_list args1, args2;

    va_start(args1, format);
    va_copy(args2, args1);

    // 获取所需长度
    const int length = std::vsnprintf(nullptr, 0, format, args1);
    va_end(args1);

    if (length < 0) {
        va_end(args2);
        return "";
    }

    // 分配缓冲区
    std::vector<char> buffer(static_cast<size_t>(length) + 1);

    // 实际格式化
    std::vsnprintf(buffer.data(), buffer.size(), format, args2);
    va_end(args2);

    return buffer.data();
}


#endif //SWITCHPROCONTROLLERESP32S3_LOG_H