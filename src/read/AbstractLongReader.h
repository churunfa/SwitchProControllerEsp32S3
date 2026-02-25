//
// Created by churunfa on 2026/2/18.
//

#ifndef SWITCHPROCONTROLLERESP32S3_ABSTRACTLONGREADER_H
#define SWITCHPROCONTROLLERESP32S3_ABSTRACTLONGREADER_H
#include "AbstractBatchReader.h"

/**
 * 报文格式：0,1-当前分片   2,3-最后一个分片的编号   4-数据长度  >5-数据
 */
class AbstractLongReader : public AbstractBatchReader {
    std::vector<uint8_t> buffer_;
public:
    void batch_exec(std::vector<uint8_t> buffer) override {
        buffer_.insert(buffer_.end(), buffer.begin(), buffer.end());
    }
    void read_finished() override {
        read_completed_exec(buffer_);
        buffer_.clear();
    }
    virtual void read_completed_exec(std::vector<uint8_t> buffer) = 0;
};

#endif //SWITCHPROCONTROLLERESP32S3_ABSTRACTLONGREADER_H