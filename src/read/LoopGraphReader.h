//
// Created by churunfa on 2026/2/16.
//

#ifndef SWITCHPROCONTROLLERESP32S3_LOOPGRAPH_H
#define SWITCHPROCONTROLLERESP32S3_LOOPGRAPH_H

#include <graph/Graph.h>
#include <debug/led_control.h>
#include <debug/log.h>

#include "AbstractLongReader.h"

class LoopGraphReader : public AbstractLongReader {
    Graph graph;
public:
    void read_completed_exec(std::vector<uint8_t> buffer) override {
        // 开始处理时显示黄色LED
        showYellowLed();
        
        if (glz::read_json(graph, buffer)) {
            logPrintf("Failed to serialize graph\n");
            showRedLed();
            return;
        }
        
        logPrintf("json=%s\n", buffer.data());
        GraphExecutor::getInstance().updateExecGraph(graph);
        
        // 成功时显示蓝色LED
        showBlueLed();
    }
};

#endif //SWITCHPROCONTROLLERESP32S3_LOOPGRAPH_H