#ifndef SWITCHPROCONTROLLERESP32S3_SIMPLECONFIG_H
#define SWITCHPROCONTROLLERESP32S3_SIMPLECONFIG_H

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>
#include <debug/log.h>
#include <vector>
#include <glaze/glaze.hpp>

// 简化的配置管理类
class SimpleConfig {
private:
    String configFile;
    std::map<std::string, glz::raw_json> configMap;
    bool loaded = false;
    
    // 确保目录存在
    void ensureDir() {
        String dir = configFile.substring(0, configFile.lastIndexOf('/'));
        if (!dir.isEmpty() && !LittleFS.exists(dir)) {
            LittleFS.mkdir(dir);
        }
    }
    
    // Base64编码
    static String encodeBase64(const uint8_t* data, size_t len) {
        static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        String result;
        int i = 0;
        int j = 0;
        uint8_t buf3[3];
        uint8_t buf4[4];
        
        while (len--) {
            buf3[i++] = *(data++);
            if (i == 3) {
                buf4[0] = (buf3[0] & 0xfc) >> 2;
                buf4[1] = ((buf3[0] & 0x03) << 4) + ((buf3[1] & 0xf0) >> 4);
                buf4[2] = ((buf3[1] & 0x0f) << 2) + ((buf3[2] & 0xc0) >> 6);
                buf4[3] = buf3[2] & 0x3f;
                
                for (i = 0; i < 4; i++) {
                    result += chars[buf4[i]];
                }
                i = 0;
            }
        }
        
        if (i) {
            for (j = i; j < 3; j++) buf3[j] = '\0';
            buf4[0] = (buf3[0] & 0xfc) >> 2;
            buf4[1] = ((buf3[0] & 0x03) << 4) + ((buf3[1] & 0xf0) >> 4);
            buf4[2] = ((buf3[1] & 0x0f) << 2) + ((buf3[2] & 0xc0) >> 6);
            buf4[3] = buf3[2] & 0x3f;
            
            for (j = 0; j < i + 1; j++) {
                result += chars[buf4[j]];
            }
            while (i++ < 3) result += '=';
        }
        return result;
    }
    
    // Base64解码
    static std::vector<uint8_t> decodeBase64(const String& str) {
        std::vector<uint8_t> result;
        int in_len = str.length();
        int i = 0, in_ = 0;
        uint8_t buf4[4], buf3[3];
        
        while (in_len-- && str[in_] != '=' && 
               (isalnum(str[in_]) || str[in_] == '+' || str[in_] == '/')) {
            buf4[i++] = str[in_]; in_++;
            if (i == 4) {
                for (i = 0; i < 4; i++) {
                    if (buf4[i] >= 'A' && buf4[i] <= 'Z') buf4[i] -= 'A';
                    else if (buf4[i] >= 'a' && buf4[i] <= 'z') buf4[i] = buf4[i] - 'a' + 26;
                    else if (buf4[i] >= '0' && buf4[i] <= '9') buf4[i] = buf4[i] - '0' + 52;
                    else if (buf4[i] == '+') buf4[i] = 62;
                    else if (buf4[i] == '/') buf4[i] = 63;
                }
                
                buf3[0] = (buf4[0] << 2) + ((buf4[1] & 0x30) >> 4);
                buf3[1] = ((buf4[1] & 0xf) << 4) + ((buf4[2] & 0x3c) >> 2);
                buf3[2] = ((buf4[2] & 0x3) << 6) + buf4[3];
                
                for (i = 0; i < 3; i++) result.push_back(buf3[i]);
                i = 0;
            }
        }
        
        if (i) {
            for (int j = i; j < 4; j++) buf4[j] = 0;
            for (int j = 0; j < 4; j++) {
                if (buf4[j] >= 'A' && buf4[j] <= 'Z') buf4[j] -= 'A';
                else if (buf4[j] >= 'a' && buf4[j] <= 'z') buf4[j] = buf4[j] - 'a' + 26;
                else if (buf4[j] >= '0' && buf4[j] <= '9') buf4[j] = buf4[j] - '0' + 52;
                else if (buf4[j] == '+') buf4[j] = 62;
                else if (buf4[j] == '/') buf4[j] = 63;
            }
            
            buf3[0] = (buf4[0] << 2) + ((buf4[1] & 0x30) >> 4);
            buf3[1] = ((buf4[1] & 0xf) << 4) + ((buf4[2] & 0x3c) >> 2);
            buf3[2] = ((buf4[2] & 0x3) << 6) + buf4[3];
            
            for (int j = 0; j < i - 1; j++) result.push_back(buf3[j]);
        }
        return result;
    }

public:
    explicit SimpleConfig(const String& filename = "/config/settings.json") 
        : configFile(filename) {
        ensureDir();
    }
    
    // 加载配置
    bool load() {
        if (!LittleFS.exists(configFile)) {
            loaded = true;
            return false;
        }
        
        File file = LittleFS.open(configFile, "r");
        if (!file) return false;
        
        size_t fileSize = file.size();
        std::string buffer(fileSize, '\0');
        file.read(reinterpret_cast<uint8_t*>(buffer.data()), fileSize);
        file.close();
        
        auto ec = glz::read_json(configMap, buffer);
        if (ec) {
            logPrintf("[SimpleConfig] JSON解析错误: %s\n", glz::format_error(ec, buffer).c_str());
            return false;
        }
        
        loaded = true;
        logPrintf("[SimpleConfig] 配置加载成功: %s\n", configFile.c_str());
        return true;
    }
    
    // 保存配置（每次修改后自动调用）
    bool save() {
        if (!loaded) return false;
        
        File file = LittleFS.open(configFile, "w");
        if (!file) return false;
        
        std::string buffer;
        auto ec = glz::write_json(configMap, buffer);
        if (ec) {
            logPrintf("[SimpleConfig] JSON序列化错误\n");
            file.close();
            return false;
        }
        
        file.write(reinterpret_cast<const uint8_t*>(buffer.data()), buffer.size());
        file.close();
        
        logPrintf("[SimpleConfig] 配置保存成功: %s\n", configFile.c_str());
        return true;
    }
    
    // 基础类型操作
    void set(const String& key, const String& value) { 
        configMap[std::string(key.c_str())] = glz::raw_json{"\"" + std::string(value.c_str()) + "\""}; 
        save(); 
    }
    void set(const String& key, int value) { 
        configMap[std::string(key.c_str())] = glz::raw_json{std::to_string(value)}; 
        save(); 
    }
    void set(const String& key, bool value) { 
        configMap[std::string(key.c_str())] = glz::raw_json{value ? "true" : "false"}; 
        save(); 
    }
    void set(const String& key, float value) { 
        configMap[std::string(key.c_str())] = glz::raw_json{std::to_string(value)}; 
        save(); 
    }
    
    String getString(const String& key, const String& defaultValue = "") {
        auto it = configMap.find(std::string(key.c_str()));
        if (it != configMap.end()) {
            std::string result;
            auto ec = glz::read_json(result, it->second.str);
            if (!ec) return String(result.c_str());
        }
        return defaultValue;
    }
    
    int getInt(const String& key, int defaultValue = 0) {
        auto it = configMap.find(std::string(key.c_str()));
        if (it != configMap.end()) {
            int result;
            auto ec = glz::read_json(result, it->second.str);
            if (!ec) return result;
        }
        return defaultValue;
    }
    
    bool getBool(const String& key, bool defaultValue = false) {
        auto it = configMap.find(std::string(key.c_str()));
        if (it != configMap.end()) {
            bool result;
            auto ec = glz::read_json(result, it->second.str);
            if (!ec) return result;
        }
        return defaultValue;
    }
    
    float getFloat(const String& key, float defaultValue = 0.0f) {
        auto it = configMap.find(std::string(key.c_str()));
        if (it != configMap.end()) {
            float result;
            auto ec = glz::read_json(result, it->second.str);
            if (!ec) return result;
        }
        return defaultValue;
    }
    
    // 二进制数据操作（自动Base64编码）
    void setBinary(const String& key, const uint8_t* data, size_t len) {
        String encoded = encodeBase64(data, len);
        configMap[std::string(key.c_str())] = glz::raw_json{"\"" + std::string(encoded.c_str()) + "\""};
        save();
    }
    
    std::vector<uint8_t> getBinary(const String& key) {
        auto it = configMap.find(std::string(key.c_str()));
        if (it != configMap.end()) {
            std::string encoded;
            auto ec = glz::read_json(encoded, it->second.str);
            if (!ec) return decodeBase64(String(encoded.c_str()));
        }
        return std::vector<uint8_t>();
    }
    
    // 数组操作
    template<typename T>
    void setArray(const String& key, const std::vector<T>& array) {
        std::string jsonStr;
        auto ec = glz::write_json(array, jsonStr);
        if (!ec) {
            configMap[std::string(key.c_str())] = glz::raw_json{jsonStr};
            save();
        }
    }
    
    template<typename T>
    std::vector<T> getArray(const String& key) {
        std::vector<T> result;
        auto it = configMap.find(std::string(key.c_str()));
        if (it != configMap.end()) {
            auto ec = glz::read_json(result, it->second.str);
            if (ec) result.clear();
        }
        return result;
    }
    
    // 实用方法
    bool contains(const String& key) { return configMap.find(std::string(key.c_str())) != configMap.end(); }
    void remove(const String& key) { configMap.erase(std::string(key.c_str())); save(); }
    void clear() { configMap.clear(); save(); }
    std::vector<String> keys() {
        std::vector<String> result;
        for (const auto& kv : configMap) {
            result.push_back(String(kv.first.c_str()));
        }
        return result;
    }
    
    void print() {
        logPrintf("[SimpleConfig] 配置内容 (%s):\n", configFile.c_str());
        std::string buffer;
        auto ec = glz::write_json(configMap, buffer);
        if (!ec) {
            Serial.println(buffer.c_str());
        } else {
            logPrintf("[SimpleConfig] JSON序列化失败\n");
        }
    }
};

// BLE配置专用类
class BleConfig {
private:
    static SimpleConfig config;
    
public:
    static void init() {
        config.load();
    }
    
    // MAC地址操作
    static void setMac(const uint8_t* mac) {
        config.setBinary("mac_address", mac, 6);
    }
    
    static bool getMac(uint8_t* mac) {
        auto data = config.getBinary("mac_address");
        if (data.size() == 6) {
            memcpy(mac, data.data(), 6);
            return true;
        }
        return false;
    }
    
    static bool hasMac() {
        return config.contains("mac_address");
    }
    
    // 广播数据操作
    static void setAdvData(const uint8_t* data, size_t len) {
        config.setBinary("adv_data", data, len);
        config.set("adv_data_len", static_cast<int>(len));
    }
    
    static bool getAdvData(uint8_t* data, size_t& len) {
        auto advData = config.getBinary("adv_data");
        if (!advData.empty()) {
            int storedLen = config.getInt("adv_data_len", advData.size());
            len = static_cast<size_t>(storedLen);
            if (len <= 31 && len <= advData.size()) {
                memcpy(data, advData.data(), len);
                return true;
            }
        }
        return false;
    }
    
    static bool hasAdvData() {
        return config.contains("adv_data") && config.contains("adv_data_len");
    }
    
    // 其他BLE配置
    static void setDeviceName(const String& name) { config.set("device_name", name); }
    static String getDeviceName(const String& defaultName = "Switch Pro Controller") {
        return config.getString("device_name", defaultName);
    }
    
    static void setTxPower(int power) { config.set("tx_power", power); }
    static int getTxPower(int defaultPower = 0) { return config.getInt("tx_power", defaultPower); }
    
    // 配置管理
    static void clear() { config.clear(); }
    static bool isComplete() { return hasMac() && hasAdvData(); }
    static void print() { config.print(); }
};

// 静态成员定义
SimpleConfig BleConfig::config("/config/ble.json");

#endif //SWITCHPROCONTROLLERESP32S3_SIMPLECONFIG_H