//
// Created by churunfa on 2026/2/4.
//

#ifndef SWITCHPROCONTROLLERESP32S3_WRITEBACKREADER_H
#define SWITCHPROCONTROLLERESP32S3_WRITEBACKREADER_H

#include "ReadStrategy.h"

class WriteBackReader : public ReadStrategy {
    uint8_t buffer[45] = {};
    uint8_t readSize = 45;
    uint8_t readyIndex = 0;

public:
    void readData(const uint8_t inByte) override {
        buffer[readyIndex++] = inByte;
    }

    void exec() override {
        Serial0.write(buffer, readSize);
        readyIndex = 0;
    }

    void reset() override {
        readyIndex = 0;
    }

    int8_t length() override {
        return 45;
    }
};

#endif //SWITCHPROCONTROLLERESP32S3_WRITEBACKREADER_H