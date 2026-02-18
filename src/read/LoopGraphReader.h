//
// Created by churunfa on 2026/2/16.
//

#ifndef SWITCHPROCONTROLLERESP32S3_LOOPGRAPH_H
#define SWITCHPROCONTROLLERESP32S3_LOOPGRAPH_H

#include <graph/Graph.h>

#include "ReadStrategy.h"

class LoopGraphReader : public ReadStrategy {
    Graph graph;
    std::string buffer;
    int byte_index = 0; // 1~3长度
    int len = 0;
public:
    void readData(const uint8_t inByte) override {
        if (byte_index < 3) {
            // 读取长度
            len = static_cast<int>(inByte) << (byte_index * 8) | len;
            byte_index++;
            logPrintf("len=%d\n", len);
            return;
        }
        buffer.push_back(inByte);
        byte_index++;
    }

    void exec() override {
        logPrintf("读取JSON=%s\n", buffer.data());
        if (glz::read_json(graph, buffer)) {
            logPrintf("Failed to serialize graph\n");
        }
        GraphExecutor::getInstance().updateExecGraph(graph);
    }

    void reset() override {
        buffer.clear();
        byte_index = 0;
        len = 0;
    }

    int length() override {
        return len + 3; // 前三个字节是长度，也需要加上
    }
};

#endif //SWITCHPROCONTROLLERESP32S3_LOOPGRAPH_H