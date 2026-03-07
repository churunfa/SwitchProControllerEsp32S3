//
// Created by churunfa on 2026/3/7.
//

#include "NotifyMessage.h"
#include <ble/NativeBLEReader.h>

void NotifyMessage::send(const NOTIFY_MESSAGE_TYPE type, const std::string &message) {
    send(type, std::vector<int8_t>(message.begin(), message.end()));
}

void NotifyMessage::send(const NOTIFY_MESSAGE_TYPE type, const std::vector<int8_t>& data) {
        // 拆包发送
        static int batch_size = 128;

        const size_t totalSize = data.size();
        const size_t numChunks = (totalSize + batch_size - 1) / batch_size; // 向上取整

        for (size_t i = 0; i < numChunks; ++i) {
            std::vector<uint8_t> packet;

            packet.push_back(0xAA);
            packet.push_back(0x55);

            // 类型
            packet.push_back(type);

            // 字节 0,1: 当前分片索引 (从 0 开始)
            const auto curChunk = static_cast<uint16_t>(i);
            packet.push_back(curChunk & 0xFF);
            packet.push_back((curChunk >> 8) & 0xFF);

            // 字节 2,3: 最后一个分片的编号
            const auto lastChunk = static_cast<uint16_t>(numChunks - 1);
            packet.push_back(lastChunk & 0xFF);
            packet.push_back((lastChunk >> 8) & 0xFF);

            // 计算当前分片的数据范围
            const size_t offset = i * batch_size;
            const size_t remaining = totalSize - offset;
            const size_t chunkLen = remaining < batch_size ? remaining : batch_size;

            // 字节 4: 数据长度
            packet.push_back(static_cast<uint8_t>(chunkLen));

            // 字节 5+: 实际数据
            for (size_t j = 0; j < chunkLen; ++j) {
                packet.push_back(static_cast<uint8_t>(data[offset + j]));
            }

            // 校验和
            uint8_t checksum = 0;
            for (int j = 3; j < packet.size(); j++) {
                checksum ^= packet[j];
            }
            packet.push_back(checksum);

            // 通过 BLE 发送 packet
            if (NativeBLEReader::getInstance().isConnected()) {
                NativeBLEReader::getInstance().send(packet);
            }
            Serial0.write(packet.data(), packet.size());
        }
    }