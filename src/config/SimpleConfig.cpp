#include "SimpleConfig.h"
#include <FS.h>
#include <LittleFS.h>
#include <debug/log.h>
#include <notify/NotifyMessage.h>

static auto TAG = "SimpleConfig";

SimpleConfig& SimpleConfig::getInstance() {
    static SimpleConfig instance;
    return instance;
}

bool SimpleConfig::initialize() {
    NotifyMessage::send(LOG, "Initializing SimpleConfig");
    
    // 挂载文件系统
    if (!LittleFS.begin(true)) {
        NotifyMessage::send(LOG, "Failed to mount LittleFS");
        return false;
    }
    
    NotifyMessage::send(LOG, "LittleFS mounted successfully");
    
    // 尝试加载现有配置
    if (loadFromFile()) {
        NotifyMessage::send(LOG, "Configuration loaded successfully");
        printAllConfigs();
        return true;
    }
    NotifyMessage::send(LOG, "No existing configuration found, starting fresh");
    return true; // 仍然返回true，因为这是正常情况
}

bool SimpleConfig::setConfig(const ConfigType type, const std::vector<uint8_t>& data) {
    if (data.empty()) {
        NotifyMessage::send(LOG, std::format("Attempted to set empty config data for type {}", static_cast<int>(type)));
        return false;
    }
    
    configData.configMap[type] = data;
    NotifyMessage::send(LOG, std::format("Set config for type {}, size: {} bytes", static_cast<int>(type), data.size()));
    
    // 自动保存到文件
    return saveToFile();
}

bool SimpleConfig::setConfig(const ConfigType type, const String& str) {
    if (str.length() == 0) {
        NotifyMessage::send(LOG, std::format("Attempted to set empty string config for type {}", static_cast<int>(type)));
        return false;
    }

    const std::vector<uint8_t> data(str.begin(), str.end());
    return setConfig(type, data);
}

bool SimpleConfig::getConfig(const ConfigType type, std::vector<uint8_t>& data) const {
    if (const auto it = configData.configMap.find(type); it != configData.configMap.end()) {
        data = it->second;
        return true;
    }
    return false;
}

String SimpleConfig::getConfigAsString(const ConfigType type) const {
    std::vector<uint8_t> data;
    if (getConfig(type, data)) {
        return {reinterpret_cast<const char*>(data.data()), data.size()};
    }
    return {};
}

bool SimpleConfig::hasConfig(const ConfigType type) const {
    return configData.configMap.contains(type);
}

bool SimpleConfig::removeConfig(const ConfigType type) {
    if (const auto it = configData.configMap.find(type); it != configData.configMap.end()) {
        configData.configMap.erase(it);
        NotifyMessage::send(LOG, std::format("Removed config for type {}", static_cast<int>(type)));
        return saveToFile();
    }
    return false;
}

void SimpleConfig::clearAllConfigs() {
    configData.configMap.clear();
    NotifyMessage::send(LOG, "Cleared all configurations");
    saveToFile();
}

size_t SimpleConfig::getConfigCount() const {
    return configData.configMap.size();
}

bool SimpleConfig::saveToFile() {
    std::string buffer{};

    if (glz::write_json(configData, buffer)) {
        NotifyMessage::send(LOG, "Failed to serialize config data to JSON");
        return false;
    }
    
    File file = LittleFS.open(configFile.c_str(), "w");
    if (!file) {
        NotifyMessage::send(LOG, std::format("Failed to open config file for writing: {}", configFile.c_str()));
        return false;
    }

    const size_t bytesWritten = file.write(reinterpret_cast<const uint8_t*>(buffer.c_str()), buffer.length());
    file.close();
    
    if (bytesWritten != buffer.length()) {
        NotifyMessage::send(LOG, "Failed to write complete config data to file");
        return false;
    }
    
    NotifyMessage::send(LOG, std::format("Configuration saved to file successfully, size: {} bytes", bytesWritten));
    return true;
}

bool SimpleConfig::loadFromFile() {
    if (!LittleFS.exists(configFile.c_str())) {
        NotifyMessage::send(LOG, std::format("Config file does not exist: {}", configFile.c_str()));
        return false;
    }
    
    File file = LittleFS.open(configFile.c_str(), "r");
    if (!file) {
        NotifyMessage::send(LOG, std::format("Failed to open config file for reading: {}", configFile.c_str()));
        return false;
    }

    const size_t fileSize = file.size();
    if (fileSize == 0) {
        NotifyMessage::send(LOG, "Config file is empty");
        file.close();
        return false;
    }
    
    std::string buffer(fileSize, '\0');
    const size_t bytesRead = file.read(reinterpret_cast<uint8_t*>(&buffer[0]), fileSize);
    file.close();
    
    if (bytesRead != fileSize) {
        NotifyMessage::send(LOG, "Failed to read complete config file");
        return false;
    }

    if (glz::read_json(configData, buffer)) {
        NotifyMessage::send(LOG, "Failed to deserialize config data from JSON");
        return false;
    }
    for (const auto& [key, value] : configData.configMap) {
        String typeName = "UNKNOWN";
        switch (key) {
            case ConfigType::MAC_ADDRESS: typeName = "MAC_ADDRESS"; break;
            case ConfigType::NS2_WAKE_DATA: typeName = "NS2_WAKE_DATA"; break;
        }

        NotifyMessage::send(LOG, std::format("Key: {} ({}), Value: ", typeName.c_str(), static_cast<int>(key)));

        for (const auto& byte : value) {
            NotifyMessage::send(LOG, string_printf("%02X ", byte));
        }
        NotifyMessage::send(LOG, "\n");
    }
    NotifyMessage::send(LOG, "Configuration loaded from file successfully");
    return true;
}

void SimpleConfig::printAllConfigs() const {
    NotifyMessage::send(LOG, "=== Current Configuration ===");
    NotifyMessage::send(LOG, std::format("Total configs: {}", getConfigCount()));
    
    for (const auto&[fst, snd] : configData.configMap) {
        String typeName = "UNKNOWN";
        switch (fst) {
            case ConfigType::MAC_ADDRESS: typeName = "MAC_ADDRESS"; break;
            case ConfigType::NS2_WAKE_DATA: typeName = "NS2_WAKE_DATA"; break;
        }

        NotifyMessage::send(LOG, std::format("Type: {} ({}), Size: {} bytes",
                 typeName.c_str(), static_cast<int>(fst), snd.size()));

        // 打印具体数据内容（前10个字节）
        if (!snd.empty()) {
            NotifyMessage::send(LOG, "Data preview (first 10 bytes): ");
            for (size_t i = 0; i < std::min(snd.size(), size_t(10)); i++) {
                NotifyMessage::send(LOG, string_printf("%02X ", snd[i]));
            }
            if (snd.size() > 10) {
                NotifyMessage::send(LOG, string_printf("... (and %d more bytes)", snd.size() - 10));
            }
        }
    }
    NotifyMessage::send(LOG, "============================");
}