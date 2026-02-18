//
// Created by churunfa on 2026/2/16.
//

#include "Graph.h"

namespace SwitchGraphExec{
    SwitchProSerialInput serialInput;
    unsigned long imu_last_collect_time = 0;

    const ImuData gravitationImuData[3] = {
        {0, 4096, 0, 0, 0, 0},
        {0, 4096, 0, 0, 0, 0},
        {0, 4096, 0, 0, 0, 0}
    };

    uint16_t standardAnalog(int x) {
        // 标准化到 -2047 ~ 2047
        x = std::min(x, 2047);
        x = std::max(x, -2047);
        // 坐标转化
        return x + 2048;
    }

    void setAnalogX(SwitchAnalog& stick, const uint16_t x) {
        uint8_t *data = stick.data;
        data[0] = x & 0xFF;
        data[1] = (data[1] & 0xF0) | ((x >> 8) & 0x0F);
    }

    void setAnalogY(SwitchAnalog& stick, const uint16_t y) {
        uint8_t *data = stick.data;
        data[1] = (data[1] & 0x0F) | ((y & 0x0F) << 4);
        data[2] = (y >> 4) & 0xFF;
    }
}

void GraphNode::runOpt(const std::string &base_operate, const std::vector<int> &param_vec, const bool reset) {
    if (base_operate == "BUTTON_Y") {
        SwitchGraphExec::serialInput.inputs.buttonY = reset;
    } else if (base_operate == "BUTTON_X") {
        SwitchGraphExec::serialInput.inputs.buttonX = reset;
    } else if (base_operate == "BUTTON_B") {
        SwitchGraphExec::serialInput.inputs.buttonB = reset;
    } else if (base_operate == "BUTTON_A") {
        SwitchGraphExec::serialInput.inputs.buttonA = reset;
    } else if (base_operate == "BUTTON_R") {
        SwitchGraphExec::serialInput.inputs.buttonR = reset;
    } else if (base_operate == "BUTTON_ZR") {
        SwitchGraphExec::serialInput.inputs.buttonZR = reset;
    } else if (base_operate == "BUTTON_MINUS") {
        SwitchGraphExec::serialInput.inputs.buttonMinus = reset;
    } else if (base_operate == "BUTTON_PLUS") {
        SwitchGraphExec::serialInput.inputs.buttonPlus = reset;
    } else if (base_operate == "BUTTON_THUMB_R") {
        SwitchGraphExec::serialInput.inputs.buttonThumbR = reset;
    } else if (base_operate == "BUTTON_THUMB_L") {
        SwitchGraphExec::serialInput.inputs.buttonThumbL = reset;
    } else if (base_operate == "BUTTON_HOME") {
        SwitchGraphExec::serialInput.inputs.buttonHome = reset;
    } else if (base_operate == "BUTTON_CAPTURE") {
        SwitchGraphExec::serialInput.inputs.buttonCapture = reset;
    } else if (base_operate == "DPAD_DOWN") {
        SwitchGraphExec::serialInput.inputs.dpadDown = reset;
    } else if (base_operate == "DPAD_UP") {
        SwitchGraphExec::serialInput.inputs.dpadUp = reset;
    } else if (base_operate == "DPAD_RIGHT") {
        SwitchGraphExec::serialInput.inputs.dpadRight = reset;
    } else if (base_operate == "DPAD_LEFT") {
        SwitchGraphExec::serialInput.inputs.dpadLeft = reset;
    } else if (base_operate == "BUTTON_L") {
        SwitchGraphExec::serialInput.inputs.buttonL = reset;
    } else if (base_operate == "BUTTON_ZL") {
        SwitchGraphExec::serialInput.inputs.buttonZL = reset;
    } else if (base_operate == "LEFT_STICK") {
        if (param_vec.size() != 2) {
            logPrintf("[ERROR]LEFT_STICK参数个数异常\n");
        } else {
            SwitchGraphExec::setAnalogX(SwitchGraphExec::serialInput.leftStick, SwitchGraphExec::standardAnalog(param_vec[0]));
            SwitchGraphExec::setAnalogY(SwitchGraphExec::serialInput.leftStick, SwitchGraphExec::standardAnalog(param_vec[1]));
        }
    } else if (base_operate == "RIGHT_STICK") {
        if (param_vec.size() != 2) {
            logPrintf("[ERROR]RIGHT_STICK参数个数异常\n");
        } else {
            SwitchGraphExec::setAnalogX(SwitchGraphExec::serialInput.rightStick, SwitchGraphExec::standardAnalog(param_vec[0]));
            SwitchGraphExec::setAnalogY(SwitchGraphExec::serialInput.rightStick, SwitchGraphExec::standardAnalog(param_vec[1]));
        }
    } else if (base_operate == "IMU") {
        if (param_vec.size() != 6) {
            logPrintf("[ERROR]IMU参数个数异常\n");
        } else {
            const unsigned long nowTime = millis();
            if (nowTime > SwitchGraphExec::imu_last_collect_time + 5) {
                for (int i = 0; i < 2; i++) {
                    SwitchGraphExec::serialInput.imuData[i].accX = SwitchGraphExec::serialInput.imuData[i + 1].accX;
                    SwitchGraphExec::serialInput.imuData[i].accY = SwitchGraphExec::serialInput.imuData[i + 1].accY;
                    SwitchGraphExec::serialInput.imuData[i].accZ = SwitchGraphExec::serialInput.imuData[i + 1].accZ;
                    SwitchGraphExec::serialInput.imuData[i].gyroX = SwitchGraphExec::serialInput.imuData[i + 1].gyroX;
                    SwitchGraphExec::serialInput.imuData[i].gyroY = SwitchGraphExec::serialInput.imuData[i + 1].gyroY;
                    SwitchGraphExec::serialInput.imuData[i].gyroZ = SwitchGraphExec::serialInput.imuData[i + 1].gyroZ;
                }
            }
            SwitchGraphExec::serialInput.imuData[2].accX = param_vec[0];
            SwitchGraphExec::serialInput.imuData[2].accY = param_vec[1];
            SwitchGraphExec::serialInput.imuData[2].accZ = param_vec[2];
            SwitchGraphExec::serialInput.imuData[2].gyroX = param_vec[3];
            SwitchGraphExec::serialInput.imuData[2].gyroY = param_vec[4];
            SwitchGraphExec::serialInput.imuData[2].gyroZ = param_vec[5];

            SwitchGraphExec::imu_last_collect_time = nowTime;
        }
    } else if (base_operate == "LEFT_STICK_CIRCLE") {
        // 暂不支持
    } else if (base_operate == "RESET_ALL") {
        std::memset(&SwitchGraphExec::serialInput, 0, sizeof(SwitchProReport));
        SwitchGraphExec::setAnalogX(SwitchGraphExec::serialInput.leftStick, 0);
        SwitchGraphExec::setAnalogY(SwitchGraphExec::serialInput.leftStick, 0);
        SwitchGraphExec::setAnalogY(SwitchGraphExec::serialInput.rightStick, 0);
        SwitchGraphExec::setAnalogY(SwitchGraphExec::serialInput.rightStick, 0);
        // 体感只留重力
        for (int i = 0; i < 3; i++) {
            SwitchGraphExec::serialInput.imuData[i] = SwitchGraphExec::gravitationImuData[i];
        }
    } else if (base_operate == "SLEEP") {
        // 空操作，不处理
    } else if (base_operate == "START_EMPTY") {
        // 空操作，不处理
    }

    SwitchProDriver::getInstance().updateInputReport(&SwitchGraphExec::serialInput);
}

void GraphNode::batchRunOpt() {
    const auto size = base_operates.size();
    if (size != params.size() || size != resets.size()) {
        logPrintf("参数个数不匹配\n");
        return;
    }
    for (int i = 0; i < size; ++i) {
        runOpt(base_operates[i], params[i], resets[i]);
    }
}

Task GraphExecutor::nodeExecCore(const std::shared_ptr<GraphNode> node) {
    bool has_auto_reset = false;
    for (auto reset_vec : node->resets) {
        if (reset_vec) {
            has_auto_reset = true;
        }
    }
    for (int i = 0; i < node->loop_cnt; i++) {
        node->batchRunOpt();
        const int exec_sleep_time = has_auto_reset ? node->exec_hold_time / 2 : node->exec_hold_time;
        const int auto_reset_sleep_time = has_auto_reset ? node->exec_hold_time / 2 : 0;
        co_await async_sleep(std::max(exec_sleep_time, 0));

        if (has_auto_reset) {
            for (int j = 0; j < node->auto_resets.size(); ++j) {
                if (node->auto_resets[j]) {
                    GraphNode::runOpt(node->base_operates[j], node->params[j], true);
                }
            }
        }
        co_await async_sleep(std::max(auto_reset_sleep_time, 0));
    }
}

Task GraphExecutor::nodeExec(const std::shared_ptr<GraphNode> node, const std::shared_ptr<std::unordered_map<int, int>> in_degrees) {
    co_await nodeExecCore(node);
    if (out_edge.contains(node->node_id)) {
        for (const auto& graph_edge : out_edge.at(node->node_id)) {
            (*in_degrees)[graph_edge->next_node_id]--;
            if ((*in_degrees)[graph_edge->next_node_id] == 0) {
                const auto next_node = node_map.at(graph_edge->next_node_id);
                nodeExec(next_node, in_degrees);
            }
        }
    }
}

void GraphExecutor::exec() {
    if (!start_node) {
        return;
    }
    const auto in_degrees = std::make_shared<std::unordered_map<int, int>>();
    for (const auto edge : exec_graph->graph_edges) {
        (*in_degrees)[edge.next_node_id]++;
    }
    nodeExec(start_node, in_degrees);

    while (!Scheduler::getInstance().isEmpty()) {
        Scheduler::getInstance().run();
        delay(1);
    }
}


GraphExecutor::GraphExecutor() {
    exec_graph = readExecGraph();
    initGraph();
    xTaskCreate(
        [](void* arg) { static_cast<GraphExecutor*>(arg)->loop(); },
        "GraphLoop",
        8192,  // 8KB 栈
        this,
        1,
        nullptr
    );
}

std::optional<Graph> GraphExecutor::readExecGraph() {
    File file = LittleFS.open("/exec_graph.dat", "r");
    if (!file) {
        logPrintf("Failed to open file for reading\n");
        return std::nullopt;
    }

    const size_t fileSize = file.size();
    std::string buffer(fileSize, '\0');
    file.read(reinterpret_cast<uint8_t*>(buffer.data()), fileSize);
    file.close();

    // 使用 Glaze 反序列化
    Graph graph;
    if (glz::read_json(graph, buffer)) {
        logPrintf("Failed to deserialize graph\n");
        return std::nullopt; // 返回空图
    }

    return graph;
}

void GraphExecutor::initGraph() {
    std::lock_guard lock(graphLock);
    if (!exec_graph) {
        return;
    }
    const auto& nodes = exec_graph->graph_nodes;
    const auto& edges = exec_graph->graph_edges;
    for (auto node : nodes) {
        if (node.base_operates.size() == 1 && node.base_operates[0] == "START_EMPTY") {
            start_node = std::make_shared<GraphNode>(node);
        }
        node_map[node.node_id] = std::make_shared<GraphNode>(node);
    }
    for (auto edge : edges) {
        edge_map[edge.edge_id] = std::make_shared<GraphEdge>(edge);
        out_edge[edge.from_node_id].push_back(std::make_shared<GraphEdge>(edge));
    }
}

void GraphExecutor::writeExecGraph(Graph& graph) {
    std::lock_guard lock(graphLock);

    // 使用 Glaze 序列化
    std::string buffer;
    if (glz::write_json(graph, buffer)) {
        logPrintf("Failed to serialize graph\n");
        return;
    }

    File file = LittleFS.open("/exec_graph.dat", "w");
    if (!file) {
        logPrintf("Failed to open file for writing\n");
        return;
    }
    file.write(reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size());
    file.close();
}

[[noreturn]] void GraphExecutor::loop() {
    while (true) {
        if (!running) {
            delay(1000);
            continue;
        }
        {
            std::lock_guard lock(graphLock);
            if (exec_graph) {
                exec();
                delay(1);
                continue;
            }
        }
        delay(1000);
    }
}

void GraphExecutor::updateExecGraph(Graph graph) {
    std::lock_guard lock(graphLock);
    exec_graph = graph;
    initGraph();
    writeExecGraph(graph);
}

void GraphExecutor::switchRunning() {
    running = !running;
}