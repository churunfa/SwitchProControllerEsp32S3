#include "SimpleConfig.h"
#include <FS.h>
#include <LittleFS.h>
#include <esp_log.h>
#include <debug/log.h>

static auto TAG = "SimpleConfig";

SimpleConfig& SimpleConfig::getInstance() {
    static SimpleConfig instance;
    return instance;
}

bool SimpleConfig::initialize() {
    ESP_LOGI(TAG, "Initializing SimpleConfig");
    
    // 挂载文件系统
    if (!LittleFS.begin(true)) {
        ESP_LOGE(TAG, "Failed to mount LittleFS");
        return false;
    }
    
    ESP_LOGI(TAG, "LittleFS mounted successfully");
    
    // 尝试加载现有配置
    if (loadFromFile()) {
        ESP_LOGI(TAG, "Configuration loaded successfully");
        printAllConfigs();
        return true;
    }
    ESP_LOGW(TAG, "No existing configuration found, starting fresh");
    return true; // 仍然返回true，因为这是正常情况
}

bool SimpleConfig::setConfig(const ConfigType type, const std::vector<uint8_t>& data) {
    if (data.empty()) {
        ESP_LOGW(TAG, "Attempted to set empty config data for type %d", static_cast<int>(type));
        return false;
    }
    
    configData.configMap[type] = data;
    ESP_LOGI(TAG, "Set config for type %d, size: %d bytes", static_cast<int>(type), data.size());
    
    // 自动保存到文件
    return saveToFile();
}

bool SimpleConfig::setConfig(const ConfigType type, const String& str) {
    if (str.length() == 0) {
        ESP_LOGW(TAG, "Attempted to set empty string config for type %d", static_cast<int>(type));
        return false;
    }

    const std::vector<uint8_t> data(str.begin(), str.end());
    return setConfig(type, data);
}

bool SimpleConfig::getConfig(ConfigType type, std::vector<uint8_t>& data) const {
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
        ESP_LOGI(TAG, "Removed config for type %d", static_cast<int>(type));
        return saveToFile();
    }
    return false;
}

void SimpleConfig::clearAllConfigs() {
    configData.configMap.clear();
    ESP_LOGI(TAG, "Cleared all configurations");
    saveToFile();
}

size_t SimpleConfig::getConfigCount() const {
    return configData.configMap.size();
}

bool SimpleConfig::saveToFile() {
    std::string buffer{};

    if (glz::write_json(configData, buffer)) {
        ESP_LOGE(TAG, "Failed to serialize config data to JSON");
        return false;
    }
    
    File file = LittleFS.open(configFile.c_str(), "w");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open config file for writing: %s", configFile.c_str());
        return false;
    }

    const size_t bytesWritten = file.write(reinterpret_cast<const uint8_t*>(buffer.c_str()), buffer.length());
    file.close();
    
    if (bytesWritten != buffer.length()) {
        ESP_LOGE(TAG, "Failed to write complete config data to file");
        return false;
    }
    
    ESP_LOGI(TAG, "Configuration saved to file successfully, size: %d bytes", bytesWritten);
    return true;
}

bool SimpleConfig::loadFromFile() {
    if (!LittleFS.exists(configFile.c_str())) {
        ESP_LOGI(TAG, "Config file does not exist: %s", configFile.c_str());
        return false;
    }
    
    File file = LittleFS.open(configFile.c_str(), "r");
    if (!file) {
        ESP_LOGE(TAG, "Failed to open config file for reading: %s", configFile.c_str());
        return false;
    }

    const size_t fileSize = file.size();
    if (fileSize == 0) {
        ESP_LOGW(TAG, "Config file is empty");
        file.close();
        return false;
    }
    
    std::string buffer(fileSize, '\0');
    const size_t bytesRead = file.read(reinterpret_cast<uint8_t*>(&buffer[0]), fileSize);
    file.close();
    
    if (bytesRead != fileSize) {
        ESP_LOGE(TAG, "Failed to read complete config file");
        return false;
    }

    if (glz::read_json(configData, buffer)) {
        ESP_LOGE(TAG, "Failed to deserialize config data from JSON");
        return false;
    }
    for (const auto& [key, value] : configData.configMap) {
        String typeName = "UNKNOWN";
        switch (key) {
            case ConfigType::MAC_ADDRESS: typeName = "MAC_ADDRESS"; break;
            case ConfigType::NS2_WAKE_DATA: typeName = "NS2_WAKE_DATA"; break;
        }

        logPrintf("Key: %s (%d), Value: ", typeName.c_str(), static_cast<int>(key));

        for (const auto& byte : value) {
            logPrintf("%02X ", byte);
        }
        logPrintf("\n");
    }
    ESP_LOGI(TAG, "Configuration loaded from file successfully");
    return true;
}

void SimpleConfig::printAllConfigs() const {
    ESP_LOGI(TAG, "=== Current Configuration ===");
    ESP_LOGI(TAG, "Total configs: %d", getConfigCount());
    
    for (const auto&[fst, snd] : configData.configMap) {
        String typeName = "UNKNOWN";
        switch (fst) {
            case ConfigType::MAC_ADDRESS: typeName = "MAC_ADDRESS"; break;
            case ConfigType::NS2_WAKE_DATA: typeName = "NS2_WAKE_DATA"; break;
        }
        
        ESP_LOGI(TAG, "Type: %s (%d), Size: %d bytes", 
                 typeName.c_str(), static_cast<int>(pair.first), pair.second.size());
        
        // 打印具体数据内容（前10个字节）
        if (!snd.empty()) {
            ESP_LOGI(TAG, "Data preview (first 10 bytes): ");
            for (size_t i = 0; i < std::min(snd.size(), size_t(10)); i++) {
                ESP_LOGI(TAG, "%02X ", pair.second[i]);
            }
            if (snd.size() > 10) {
                ESP_LOGI(TAG, "... (and %d more bytes)", pair.second.size() - 10);
            }
        }
    }
    ESP_LOGI(TAG, "============================");
}