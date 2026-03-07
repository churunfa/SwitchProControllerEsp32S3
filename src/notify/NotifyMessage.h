//
// Created by churunfa on 2026/3/7.
//

#ifndef SWITCHPROCONTROLLERESP32S3_NOTIFYSTRATEGYPROCESS_H
#define SWITCHPROCONTROLLERESP32S3_NOTIFYSTRATEGYPROCESS_H

#include <Arduino.h>
#include <string>
#include <vector>

typedef enum {
    VERSION = 0x01,
    LOG = 0x02,
} NOTIFY_MESSAGE_TYPE;

class NotifyMessage {
public:
    static NotifyMessage& getInstance() {
        static NotifyMessage instance;
        return instance;
    }
    // 禁止拷贝
    NotifyMessage(const NotifyMessage&) = delete;
    NotifyMessage& operator=(const NotifyMessage&) = delete;


    static void send(NOTIFY_MESSAGE_TYPE type, const std::string &message);

    /**
     * 校验码：0xAA, 0x55
     * 类型：0x01-版本信息  0x02-日志信息
     * 报文内容：0,1-当前分片   2,3-最后一个分片的编号   4-数据长度  >5-数据
     * 校验和：报文内容进行^
     */
    static void send(NOTIFY_MESSAGE_TYPE type, const std::vector<int8_t>& data);

private:
    NotifyMessage();
};

#endif //SWITCHPROCONTROLLERESP32S3_NOTIFYSTRATEGYPROCESS_H