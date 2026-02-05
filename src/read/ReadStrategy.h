//
// Created by churunfa on 2026/2/4.
//

#ifndef SWITCHPROCONTROLLERESP32S3_READSTRATEGY_H
#define SWITCHPROCONTROLLERESP32S3_READSTRATEGY_H

class ReadStrategy {
public:
    virtual ~ReadStrategy() = default;
    // 具体的Read接口，返回是否read结束
    virtual void readData(uint8_t inByte);
    virtual void exec();
    virtual void reset();
    virtual int8_t length();
};

#endif //SWITCHPROCONTROLLERESP32S3_READSTRATEGY_H