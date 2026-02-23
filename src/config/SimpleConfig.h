#ifndef SIMPLE_CONFIG_H
#define SIMPLE_CONFIG_H

#include <Arduino.h>
#include <map>
#include <vector>
#include <cstdint>
#include <memory>
#include <glaze/glaze.hpp>

// 配置类型枚举
enum class ConfigType {
    MAC_ADDRESS,
    NS2_WAKE_DATA
};

// 为ConfigType提供JSON序列化支持
template <>
struct glz::meta<ConfigType> {
    static constexpr std::string_view name = "ConfigType";
    using enum ConfigType;
    static constexpr auto value = enumerate(
        "MAC_ADDRESS", MAC_ADDRESS,
        "NS2_WAKE_DATA", NS2_WAKE_DATA
    );
};

// 配置数据结构
struct ConfigData {
    std::map<ConfigType, std::vector<uint8_t>> configMap;
};

// 为ConfigData提供JSON序列化支持
template <>
struct glz::meta<ConfigData> {
    using T = ConfigData;
    static constexpr auto value = object("configMap", &T::configMap);
};

class SimpleConfig {
private:
    // 配置数据
    ConfigData configData;
    
    // 配置文件路径
    const String configFile = "/config.json";
    
    // 私有构造函数
    SimpleConfig() = default;
    
    // 文件操作
    bool saveToFile();
    bool loadFromFile();
    
public:
    // 删除拷贝构造函数和赋值操作符
    SimpleConfig(const SimpleConfig&) = delete;
    SimpleConfig& operator=(const SimpleConfig&) = delete;
    
    // 获取单例实例
    static SimpleConfig& getInstance();
    
    // 初始化配置（启动时调用）
    bool initialize();
    
    // 配置操作方法
    bool setConfig(ConfigType type, const std::vector<uint8_t>& data);
    bool setConfig(ConfigType type, const String& str);
    bool getConfig(ConfigType type, std::vector<uint8_t>& data) const;
    String getConfigAsString(ConfigType type) const;
    bool hasConfig(ConfigType type) const;
    bool removeConfig(ConfigType type);
    
    // 批量操作
    void clearAllConfigs();
    size_t getConfigCount() const;
    
    // 调试方法
    void printAllConfigs() const;
};

#endif // SIMPLE_CONFIG_H