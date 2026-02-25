//
// Created by churunfa on 2026/2/4.
//

#ifndef SWITCHPROCONTROLLERESP32S3_READSTRATEGYPROCESS_H
#define SWITCHPROCONTROLLERESP32S3_READSTRATEGYPROCESS_H

#include "ReadStrategy.h"
#include "SwitchReportReader.h"
#include "WriteBackReader.h"
#include "LoopGraphReader.h"
#include "AdvDataAndMacWriter.h"
#include "SimpleOperateReader.h"
#include "ota/OtaReader.h"

class ReadStrategyProcess {
    static constexpr int MAX_STRATEGIES = 6;
    std::unique_ptr<ReadStrategy> strategies[MAX_STRATEGIES];
    ReadStrategyProcess() {
        strategies[0] = std::make_unique<SwitchReportReader>();
        strategies[1] = std::make_unique<WriteBackReader>();
        strategies[2] = std::make_unique<LoopGraphReader>();
        strategies[3] = std::make_unique<AdvDataAndMacWriter>();
        strategies[4] = std::make_unique<SimpleOperateReader>();
        strategies[5] = std::make_unique<OtaReader>();
        reset();
    }
    uint8_t header[2] = {0xAA, 0x55};

    int readIndex = 0;
    uint8_t curType{};
    uint8_t verifyCheckSum = 0;

    void reset() {
        readIndex = 0;
        verifyCheckSum = 0;
    }

public:
    void process(const uint8_t inByte) {
        const int curIndex = readIndex;
        readIndex++;
        if (curIndex < 2) {
            // 没读到头的话从头重读
            if (inByte != header[curIndex]) {
                logPrintf("read error,inByte=%02x,header=%02x,err=header读取类型异常\n", inByte, header[curIndex]);
                reset();
            }
        } else if (curIndex == 2) {
            // 读类型
            curType = inByte;
            verifyCheckSum = inByte;
        } else if (curIndex >= 3) {
            // 正在读数据
            const auto& strategy = strategies[curType];
            if (const int length = strategy->length(); curIndex < length + 3) {
                // Serial0.printf("read data=%2x\n",inByte);
                // 读数据
                verifyCheckSum ^= inByte;
                strategy->readData(inByte);
            } else if (curIndex == length + 3) {
                // 校验和
                if (verifyCheckSum == inByte) {
                    // 校验和校验成功
                    try {
                        strategy->exec();
                    } catch (const std::system_error& e) {
                        logPrintf("run error, strategy=%d,err=%s\n",curType,e.what());
                    }
                    resetLed();
                } else {
                    // 校验失败
                    logPrintf("run error, strategy=%d,inByte=%d,verifyCheckSum=%d, curIndex=%d,err=校验和校验失败\n",curType, inByte, verifyCheckSum, curIndex);
                }
                strategy->reset();
                reset();
            }
        }
    }
    static ReadStrategyProcess& getInstance();
    ReadStrategyProcess(const ReadStrategyProcess&) = delete;
    void operator=(const ReadStrategyProcess&) = delete;
};

inline ReadStrategyProcess& ReadStrategyProcess::getInstance() {
    static ReadStrategyProcess instance;
    return instance;
}

#endif //SWITCHPROCONTROLLERESP32S3_READSTRATEGYPROCESS_H