#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <map>
#include <vector>
#include <variant>
#include <fstream>
#include <iostream>

class ConfigManager {
public:
    using ConfigValue = std::variant<bool, int, double, std::string, std::vector<std::string>>;
    
    struct ConfigOption {
        std::string name;
        std::string description;
        ConfigValue defaultValue;
        ConfigValue currentValue;
        bool isAdvanced = false;
        std::vector<std::string> validValues; // For enum-like options
    };
    
    static ConfigManager& getInstance();
    
    // Configuration loading and saving
    bool loadFromFile(const std::string& filename);
    bool saveToFile(const std::string& filename) const;
    void loadFromEnvironment();
    void loadFromCommandLine(int argc, char* argv[]);
    
    // Value access
    template<typename T>
    T get(const std::string& key) const;
    
    template<typename T>
    void set(const std::string& key, const T& value);
    
    bool hasKey(const std::string& key) const;
    void resetToDefault(const std::string& key);
    void resetAllToDefault();
    
    // Configuration options registration
    void registerOption(const std::string& name, const std::string& description,
                       const ConfigValue& defaultValue, bool isAdvanced = false);
    void registerEnumOption(const std::string& name, const std::string& description,
                           const ConfigValue& defaultValue, const std::vector<std::string>& validValues,
                           bool isAdvanced = false);
    
    // Help and documentation
    void printHelp() const;
    void printAdvancedOptions() const;
    void printCurrentConfig() const;
    void validateConfiguration() const;
    
    // Configuration categories
    std::vector<std::string> getCategories() const;
    std::vector<std::string> getOptionsInCategory(const std::string& category) const;

private:
    ConfigManager() = default;
    
    std::map<std::string, ConfigOption> options_;
    std::map<std::string, std::string> categories_; // key -> category
    std::vector<std::string> commandLineArgs_;
    
    // Internal helpers
    void setDefaultValue(const std::string& key);
    std::string valueToString(const ConfigValue& value) const;
    ConfigValue stringToValue(const std::string& str, const ConfigValue& defaultValue) const;
    std::string getConfigCategory(const std::string& key) const;
    
    // Built-in configuration options
    void registerBuiltInOptions();
};

// Template implementations
template<typename T>
T ConfigManager::get(const std::string& key) const {
    auto it = options_.find(key);
    if (it == options_.end()) {
        throw std::runtime_error("Configuration key not found: " + key);
    }
    
    try {
        return std::get<T>(it->second.currentValue);
    } catch (const std::bad_variant_access&) {
        throw std::runtime_error("Type mismatch for configuration key: " + key);
    }
}

template<typename T>
void ConfigManager::set(const std::string& key, const T& value) {
    auto it = options_.find(key);
    if (it == options_.end()) {
        throw std::runtime_error("Configuration key not found: " + key);
    }
    
    // Check if value is valid for enum options
    if (!it->second.validValues.empty()) {
        std::string valueStr = valueToString(ConfigValue(value));
        if (std::find(it->second.validValues.begin(), it->second.validValues.end(), valueStr) 
            == it->second.validValues.end()) {
            throw std::runtime_error("Invalid value for key " + key + ": " + valueStr);
        }
    }
    
    it->second.currentValue = ConfigValue(value);
}

// Convenience macros
#define CONFIG_GET(key) ConfigManager::getInstance().get<decltype(ConfigManager::getInstance().get(key))>(key)
#define CONFIG_SET(key, value) ConfigManager::getInstance().set(key, value)

#endif // CONFIG_MANAGER_H
