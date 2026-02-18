//
// Created by churunfa on 2026/2/18.
//

#ifndef SWITCHPROCONTROLLERESP32S3_ABSTRACTLONGREADER_H
#define SWITCHPROCONTROLLERESP32S3_ABSTRACTLONGREADER_H

/**
 * 报文格式：0,1-当前分片   2,3-最后一个分片的编号   4-数据长度  >5-数据
 */
class AbstractLongReader : public ReadStrategy {
    uint16_t cur_shard = 0;
    uint16_t last_shard_index = 0;
    int len = 0;
    int byte_index = 0;

    std::vector<uint8_t> buffer;
public:
    void readData(const uint8_t inByte) override {
        if (byte_index == 0) {
            cur_shard = inByte;
        } else if (byte_index == 1) {
            cur_shard = cur_shard | inByte << 8;
        } else if (byte_index == 2) {
            last_shard_index = inByte;
        } else if (byte_index == 3) {
            last_shard_index = last_shard_index | inByte << 8;
        } else if (byte_index == 4) {
            len = inByte;
        } else if (byte_index > 4) {
            buffer.push_back(inByte);
        }
        byte_index++;
    }

    void exec() override {
        if (cur_shard == last_shard_index) {
            read_completed_exec(buffer);
            buffer.clear();
        }
    }

    void reset() override {
        cur_shard = 0;
        last_shard_index = 0;
        len = 0;
        byte_index=0;
    }

    int length() override {
        return len + 5;
    }
    virtual void read_completed_exec(std::vector<uint8_t> buffer) = 0;
};

#endif //SWITCHPROCONTROLLERESP32S3_ABSTRACTLONGREADER_H