//
// Created by churunfa on 2026/2/25.
//

#ifndef SWITCHPROCONTROLLERESP32S3_OTAUTILS_H
#define SWITCHPROCONTROLLERESP32S3_OTAUTILS_H

#include <Update.h>

class OtaUtils {
public:
    // 1. 准备阶段：开始接收数据前
    static void startOTA(const size_t firmware_size = UPDATE_SIZE_UNKNOWN) {
        // 如果知道文件总大小，填入固件大小；不知道则用 UPDATE_SIZE_UNKNOWN
        if (!Update.begin(firmware_size)) {
            Update.printError(Serial0);
        } else {
            logPrintf("OTA Update Started\n");
        }
    }

    // 2. 接收数据阶段：收到每包数据时写入
    static void writeOTAData(uint8_t *data, const size_t len) {
        if (Update.write(data, len) != len) {
            Update.printError(Serial0);
            logPrintf("写入固件数据失败！\n");
        }
    }

    // 3. 结束阶段：全部数据接收完成后
    static void finishOTA() {
        // true 表示结束并尝试校验更新
        if (Update.end(true)) {
            logPrintf("OTA 升级成功！即将重启...\n");
            ESP.restart(); // 重启并进入新固件
        } else {
            Update.printError(Serial0);
            logPrintf("OTA 升级失败，请重试！\n");
        }
    }

};
#endif //SWITCHPROCONTROLLERESP32S3_OTAUTILS_H