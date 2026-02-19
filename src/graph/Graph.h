//
// Created by churunfa on 2026/2/16.
//

#ifndef SWITCHPROCONTROLLERESP32S3_GRAPH_H
#define SWITCHPROCONTROLLERESP32S3_GRAPH_H

#include <map>
#include <thread>
#include <optional>
#include <Arduino.h>
#include <mutex>

#include <FS.h>
#include <LittleFS.h>
#include <debug/log.h>
#include <switch/driver/SwitchProDriver.h>

#include "glaze/glaze.hpp"
#include "Coroutine.h"

struct GraphNode {
    int node_id{};
    std::vector<std::string> base_operates;
    std::vector<std::vector<int>> params;
    std::vector<bool> resets;
    std::vector<bool> auto_resets;
    int exec_hold_time{};
    int loop_cnt{};

    // 构造函数
    GraphNode() = default;

    static void runOpt(const std::string &base_operate, const std::vector<int> &param_vec, bool reset);
    void batchRunOpt();
};

template <>
struct glz::meta<GraphNode> {
    static constexpr auto value = object(
       "node_id", &GraphNode::node_id,
       "base_operates", &GraphNode::base_operates,
       "params", &GraphNode::params,
       "resets", &GraphNode::resets,
       "auto_resets", &GraphNode::auto_resets,
       "exec_hold_time", &GraphNode::exec_hold_time,
       "loop_cnt", &GraphNode::loop_cnt
    );
};

struct GraphEdge {
    int edge_id{};
    int from_node_id{};
    int next_node_id{};

    // 构造函数
    GraphEdge() = default;
};

template <>
struct glz::meta<GraphEdge> {
    static constexpr auto value = object(
       "edge_id", &GraphEdge::edge_id,
       "from_node_id", &GraphEdge::from_node_id,
       "next_node_id", &GraphEdge::next_node_id
    );
};

struct Graph {
    int id{};
    std::vector<GraphNode> graph_nodes;
    std::vector<GraphEdge> graph_edges;
};

template <>
struct glz::meta<Graph> {
    static constexpr auto value = object(
       "id", &Graph::id,
       "graph_nodes", &Graph::graph_nodes,
       "graph_edges", &Graph::graph_edges
    );
};

extern const std::map<int, Graph> GLOBAL_GRAPH_MAP;

class GraphExecutor {
    std::optional<Graph> exec_graph;
    std::recursive_mutex graphLock;

    std::thread exec_thread_;
    std::recursive_mutex switch_running_lock;
    std::shared_ptr<GraphNode> start_node;
    std::map<int, std::shared_ptr<GraphNode>> node_map;
    std::map<int, std::shared_ptr<GraphEdge>> edge_map;
    std::map<int, std::vector<std::shared_ptr<GraphEdge>>> out_edge;

    GraphExecutor();
    static std::optional<Graph> readExecGraph();
    void writeExecGraph(Graph& graph);
    void initGraph();
    [[noreturn]] void loop();
    static Task nodeExecCore(std::shared_ptr<GraphNode> node);
    Task nodeExec(std::shared_ptr<GraphNode> node, std::shared_ptr<std::unordered_map<int, int>> in_degrees);
    void exec();
public:
    std::atomic<bool> running{false};
    void updateExecGraph(Graph graph);
    void switchRunning();
    static void connectGamepad();
    static GraphExecutor& getInstance() {
        static GraphExecutor instance;
        return instance;
    }
    // 禁止拷贝
    GraphExecutor(const GraphExecutor&) = delete;
    GraphExecutor& operator=(const GraphExecutor&) = delete;
};

#endif //SWITCHPROCONTROLLERESP32S3_GRAPH_H