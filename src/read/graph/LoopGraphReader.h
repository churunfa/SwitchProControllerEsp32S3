//
// Created by churunfa on 2026/2/16.
//

#ifndef SWITCHPROCONTROLLERESP32S3_LOOPGRAPH_H
#define SWITCHPROCONTROLLERESP32S3_LOOPGRAPH_H

#include <graph/Graph.h>
#include <debug/log.h>

#include "../AbstractLongReader.h"

class LoopGraphReader : public AbstractLongReader {
    Graph graph;
public:
    void read_completed_exec(std::vector<uint8_t> buffer) override {
        if (glz::read_json(graph, buffer)) {
            logPrintf("Failed to serialize graph\n");
            return;
        }
        
        logPrintf("json=%s\n", buffer.data());
        GraphExecutor::getInstance().updateExecGraph(graph);
    }
};

#endif //SWITCHPROCONTROLLERESP32S3_LOOPGRAPH_H