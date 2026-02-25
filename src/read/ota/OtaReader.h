//
// Created by churunfa on 2026/2/25.
//

#ifndef SWITCHPROCONTROLLERESP32S3_OTAREADER_H
#define SWITCHPROCONTROLLERESP32S3_OTAREADER_H
#include "OtaUtils.h"

class OtaReader : public AbstractBatchReader {
    int cur = 0;

    void batch_exec(std::vector<uint8_t> buffer) override {
        if (cur == 0) {
            OtaUtils::startOTA();
        }
        OtaUtils::writeOTAData(buffer.data(), buffer.size());
        cur++;
    }
    void read_finished() override {
        OtaUtils::finishOTA();
        cur = 0;
    }
};

#endif //SWITCHPROCONTROLLERESP32S3_OTAREADER_H