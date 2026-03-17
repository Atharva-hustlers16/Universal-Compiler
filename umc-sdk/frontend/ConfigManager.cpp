#include "ConfigManager.h"
#include <algorithm>
#include <sstream>
#include <stdexcept>

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

void ConfigManager::registerBuiltInOptions() {
    // General options
    registerOption("verbose", "Enable verbose output", false);
    registerOption("quiet", "Suppress all output", false);
    registerOption("log_file", "Log file path", std::string("umc.log"));
    registerOption("config_file", "Configuration file path", std::string("umc.conf"));
    
    // Frontend options
    registerOption("language", "Target language", std::string("auto"));
    registerEnumOption("parser", "Parser backend", std::string("enhanced"), 
                      {"basic", "enhanced", "tree-sitter", "antlr4"});
    registerOption("strict_mode", "Enable strict parsing", false);
    registerOption("show_ast", "Display generated AST", false);
    registerOption("ast_format", "AST output format", std::string("text"));
    
    // Optimization options
    registerOption("optimize", "Enable optimizations", true);
    registerOption("opt_level", "Optimization level", 1);
    registerOption("constant_folding", "Enable constant folding", true);
    registerOption("dead_code_elimination", "Enable dead code elimination", true);
    registerOption("function_inlining", "Enable function inlining", false);
    registerOption("algebraic_simplification", "Enable algebraic simplification", true);
    
    // Runtime options
    registerOption("run", "Execute AST after parsing", false);
    registerOption("debug_mode", "Enable debug mode", false);
    registerOption("memory_limit", "Memory limit in MB", 1024);
    registerOption("timeout", "Execution timeout in seconds", 30);
    
    // LLVM options
    registerOption("enable_llvm", "Enable LLVM backend", true);
    registerOption("llvm_opt_level", "LLVM optimization level", 2);
    registerOption("emit_llvm_ir", "Emit LLVM IR", false);
    registerOption("llvm_target", "LLVM target triple", std::string(""));
    
    // Output options
    registerOption("output_file", "Output file path", std::string("output"));
    registerOption("output_format", "Output format", std::string("executable"));
    registerEnumOption("output_format", "Output format", std::string("executable"),
                      {"executable", "object", "assembly", "llvm-ir", "ast"});
    
    // Advanced options
    registerOption("tree_sitter_path", "Tree-sitter library path", std::string(""), true);
    registerOption("antlr4_path", "ANTLR4 runtime path", std::string(""), true);
    registerOption("llvm_path", "LLVM installation path", std::string(""), true);
    registerOption("plugin_path", "Plugin directory path", std::string("plugins"), true);
    
    // Development options
    registerOption("debug_parser", "Debug parser operations", false, true);
    registerOption("debug_optimizer", "Debug optimizer operations", false, true);
    registerOption("profile", "Enable performance profiling", false, true);
    registerOption("benchmark", "Run performance benchmarks", false, true);
}

bool ConfigManager::loadFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Warning: Could not open config file: " << filename << std::endl;
        return false;
    }
    
    std::string line;
    int lineNumber = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        
        // Skip comments and empty lines
        if (line.empty() || line[0] == '#' || line[0] == ';') {
            continue;
        }
        
        // Parse key=value format
        size_t pos = line.find('=');
        if (pos == std::string::npos) {
            std::cerr << "Invalid config line " << lineNumber << ": " << line << std::endl;
            continue;
        }
        
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        
        // Trim whitespace
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);
        
        if (!hasKey(key)) {
            std::cerr << "Unknown configuration option: " << key << std::endl;
            continue;
        }
        
        try {
            auto& option = options_[key];
            option.currentValue = stringToValue(value, option.defaultValue);
        } catch (const std::exception& e) {
            std::cerr << "Error parsing config option " << key << ": " << e.what() << std::endl;
        }
    }
    
    file.close();
    return true;
}

bool ConfigManager::saveToFile(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error: Could not write to config file: " << filename << std::endl;
        return false;
    }
    
    file << "# UMC-SDK Configuration File\n";
    file << "# Generated automatically\n\n";
    
    // Group by category
    std::map<std::string, std::vector<std::pair<std::string, ConfigOption>>> categorized;
    for (const auto& [key, option] : options_) {
        std::string category = getConfigCategory(key);
        categorized[category].emplace_back(key, option);
    }
    
    for (const auto& [category, opts] : categorized) {
        file << "# " << category << "\n";
        for (const auto& [key, option] : opts) {
            file << "# " << option.description << "\n";
            file << key << "=" << valueToString(option.currentValue) << "\n\n";
        }
    }
    
    file.close();
    return true;
}

void ConfigManager::loadFromEnvironment() {
    // Load configuration from environment variables
    // Environment variables should be prefixed with UMC_
    const std::string prefix = "UMC_";
    
    for (auto& [key, option] : options_) {
        // Convert key to uppercase and replace dots with underscores
        std::string envKey = prefix + key;
        std::transform(envKey.begin(), envKey.end(), envKey.begin(), ::toupper);
        std::replace(envKey.begin(), envKey.end(), '.', '_');
        
        const char* envValue = std::getenv(envKey.c_str());
        if (envValue) {
            try {
                option.currentValue = stringToValue(std::string(envValue), option.defaultValue);
            } catch (const std::exception& e) {
                std::cerr << "Error parsing environment variable " << envKey << ": " << e.what() << std::endl;
            }
        }
    }
}

void ConfigManager::loadFromCommandLine(int argc, char* argv[]) {
    commandLineArgs_.clear();
    for (int i = 1; i < argc; ++i) {
        commandLineArgs_.push_back(argv[i]);
    }
    
    for (size_t i = 0; i < commandLineArgs_.size(); ++i) {
        const std::string& arg = commandLineArgs_[i];
        
        // Handle --option=value format
        if (arg.substr(0, 2) == "--") {
            size_t pos = arg.find('=');
            if (pos != std::string::npos) {
                std::string key = arg.substr(2, pos - 2);
                std::string value = arg.substr(pos + 1);
                
                if (hasKey(key)) {
                    try {
                        auto& option = options_[key];
                        option.currentValue = stringToValue(value, option.defaultValue);
                    } catch (const std::exception& e) {
                        std::cerr << "Error parsing command line option " << key << ": " << e.what() << std::endl;
                    }
                }
            } else {
                // Handle --option value format
                std::string key = arg.substr(2);
                if (hasKey(key) && i + 1 < commandLineArgs_.size()) {
                    std::string value = commandLineArgs_[i + 1];
                    
                    // Check if next argument is an option (starts with --)
                    if (value.substr(0, 2) != "--") {
                        try {
                            auto& option = options_[key];
                            option.currentValue = stringToValue(value, option.defaultValue);
                            ++i; // Skip the value argument
                        } catch (const std::exception& e) {
                            std::cerr << "Error parsing command line option " << key << ": " << e.what() << std::endl;
                        }
                    } else if (option.defaultValue.type() == typeid(bool)) {
                        // Boolean flag without value
                        options_[key].currentValue = ConfigValue(true);
                    }
                } else if (hasKey(key) && options_[key].defaultValue.type() == typeid(bool)) {
                    // Boolean flag
                    options_[key].currentValue = ConfigValue(true);
                }
            }
        }
        // Handle short options like -v, -q
        else if (arg.substr(0, 1) == "-" && arg.length() > 1) {
            std::string key;
            
            // Map common short options to full keys
            if (arg == "-v") key = "verbose";
            else if (arg == "-q") key = "quiet";
            else if (arg == "-r") key = "run";
            else if (arg == "-O") key = "optimize";
            else if (arg == "-g") key = "debug_mode";
            else key = arg.substr(1);
            
            if (hasKey(key) && options_[key].defaultValue.type() == typeid(bool)) {
                options_[key].currentValue = ConfigValue(true);
            }
        }
    }
}

bool ConfigManager::hasKey(const std::string& key) const {
    return options_.find(key) != options_.end();
}

void ConfigManager::resetToDefault(const std::string& key) {
    auto it = options_.find(key);
    if (it != options_.end()) {
        it->second.currentValue = it->second.defaultValue;
    }
}

void ConfigManager::resetAllToDefault() {
    for (auto& [key, option] : options_) {
        option.currentValue = option.defaultValue;
    }
}

void ConfigManager::registerOption(const std::string& name, const std::string& description,
                                   const ConfigValue& defaultValue, bool isAdvanced) {
    ConfigOption option;
    option.name = name;
    option.description = description;
    option.defaultValue = defaultValue;
    option.currentValue = defaultValue;
    option.isAdvanced = isAdvanced;
    options_[name] = option;
}

void ConfigManager::registerEnumOption(const std::string& name, const std::string& description,
                                       const ConfigValue& defaultValue, const std::vector<std::string>& validValues,
                                       bool isAdvanced) {
    registerOption(name, description, defaultValue, isAdvanced);
    options_[name].validValues = validValues;
}

void ConfigManager::printHelp() const {
    std::cout << "UMC-SDK Universal Compiler\n";
    std::cout << "Usage: ucc [options] <input-file>\n\n";
    
    std::cout << "General Options:\n";
    for (const auto& [key, option] : options_) {
        if (!option.isAdvanced && getConfigCategory(key) == "General") {
            std::cout << "  --" << key << "  (" << valueToString(option.currentValue) << ")\n";
            std::cout << "      " << option.description << "\n\n";
        }
    }
    
    std::cout << "Frontend Options:\n";
    for (const auto& [key, option] : options_) {
        if (!option.isAdvanced && getConfigCategory(key) == "Frontend") {
            std::cout << "  --" << key << "  (" << valueToString(option.currentValue) << ")\n";
            std::cout << "      " << option.description << "\n\n";
        }
    }
    
    std::cout << "Optimization Options:\n";
    for (const auto& [key, option] : options_) {
        if (!option.isAdvanced && getConfigCategory(key) == "Optimization") {
            std::cout << "  --" << key << "  (" << valueToString(option.currentValue) << ")\n";
            std::cout << "      " << option.description << "\n\n";
        }
    }
    
    std::cout << "Use --help-advanced to see advanced options.\n";
}

void ConfigManager::printAdvancedOptions() const {
    std::cout << "Advanced Options:\n";
    for (const auto& [key, option] : options_) {
        if (option.isAdvanced) {
            std::cout << "  --" << key << "  (" << valueToString(option.currentValue) << ")\n";
            std::cout << "      " << option.description << "\n";
            if (!option.validValues.empty()) {
                std::cout << "      Valid values: ";
                for (size_t i = 0; i < option.validValues.size(); ++i) {
                    if (i > 0) std::cout << ", ";
                    std::cout << option.validValues[i];
                }
                std::cout << "\n";
            }
            std::cout << "\n";
        }
    }
}

void ConfigManager::printCurrentConfig() const {
    std::cout << "Current Configuration:\n";
    std::cout << "=====================\n\n";
    
    // Group by category
    std::map<std::string, std::vector<std::pair<std::string, ConfigOption>>> categorized;
    for (const auto& [key, option] : options_) {
        std::string category = getConfigCategory(key);
        categorized[category].emplace_back(key, option);
    }
    
    for (const auto& [category, opts] : categorized) {
        std::cout << category << ":\n";
        for (const auto& [key, option] : opts) {
            std::cout << "  " << key << " = " << valueToString(option.currentValue);
            if (option.currentValue != option.defaultValue) {
                std::cout << " (default: " << valueToString(option.defaultValue) << ")";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}

void ConfigManager::validateConfiguration() const {
    std::vector<std::string> errors;
    
    for (const auto& [key, option] : options_) {
        // Validate enum options
        if (!option.validValues.empty()) {
            std::string currentStr = valueToString(option.currentValue);
            if (std::find(option.validValues.begin(), option.validValues.end(), currentStr) 
                == option.validValues.end()) {
                errors.push_back("Invalid value for " + key + ": " + currentStr);
            }
        }
        
        // Validate numeric ranges
        if (key == "opt_level" || key == "llvm_opt_level") {
            int level = std::get<int>(option.currentValue);
            if (level < 0 || level > 3) {
                errors.push_back("Invalid optimization level for " + key + ": " + std::to_string(level));
            }
        }
        
        if (key == "memory_limit") {
            int limit = std::get<int>(option.currentValue);
            if (limit <= 0) {
                errors.push_back("Invalid memory limit for " + key + ": " + std::to_string(limit));
            }
        }
        
        if (key == "timeout") {
            int timeout = std::get<int>(option.currentValue);
            if (timeout <= 0) {
                errors.push_back("Invalid timeout for " + key + ": " + std::to_string(timeout));
            }
        }
    }
    
    if (!errors.empty()) {
        std::cerr << "Configuration validation errors:\n";
        for (const auto& error : errors) {
            std::cerr << "  " << error << "\n";
        }
        throw std::runtime_error("Configuration validation failed");
    }
}

std::string ConfigManager::getConfigCategory(const std::string& key) const {
    // Determine category based on key prefix
    if (key.find("verbose") != std::string::npos || key.find("quiet") != std::string::npos ||
        key.find("log") != std::string::npos || key.find("config") != std::string::npos) {
        return "General";
    }
    if (key.find("language") != std::string::npos || key.find("parser") != std::string::npos ||
        key.find("strict") != std::string::npos || key.find("ast") != std::string::npos) {
        return "Frontend";
    }
    if (key.find("opt") != std::string::npos || key.find("constant") != std::string::npos ||
        key.find("dead") != std::string::npos || key.find("function") != std::string::npos ||
        key.find("algebraic") != std::string::npos) {
        return "Optimization";
    }
    if (key.find("run") != std::string::npos || key.find("debug") != std::string::npos ||
        key.find("memory") != std::string::npos || key.find("timeout") != std::string::npos) {
        return "Runtime";
    }
    if (key.find("llvm") != std::string::npos || key.find("emit") != std::string::npos) {
        return "LLVM";
    }
    if (key.find("output") != std::string::npos) {
        return "Output";
    }
    if (key.find("tree_sitter") != std::string::npos || key.find("antlr4") != std::string::npos ||
        key.find("plugin") != std::string::npos) {
        return "Advanced";
    }
    if (key.find("debug_") != std::string::npos || key.find("profile") != std::string::npos ||
        key.find("benchmark") != std::string::npos) {
        return "Development";
    }
    
    return "General";
}

std::string ConfigManager::valueToString(const ConfigValue& value) const {
    if (std::holds_alternative<bool>(value)) {
        return std::get<bool>(value) ? "true" : "false";
    }
    if (std::holds_alternative<int>(value)) {
        return std::to_string(std::get<int>(value));
    }
    if (std::holds_alternative<double>(value)) {
        return std::to_string(std::get<double>(value));
    }
    if (std::holds_alternative<std::string>(value)) {
        return std::get<std::string>(value);
    }
    if (std::holds_alternative<std::vector<std::string>>(value)) {
        auto vec = std::get<std::vector<std::string>>(value);
        std::string result;
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) result += ",";
            result += vec[i];
        }
        return result;
    }
    return "";
}

ConfigManager::ConfigValue ConfigManager::stringToValue(const std::string& str, const ConfigValue& defaultValue) const {
    if (std::holds_alternative<bool>(defaultValue)) {
        std::string lowerStr = str;
        std::transform(lowerStr.begin(), lowerStr.end(), lowerStr.begin(), ::tolower);
        if (lowerStr == "true" || lowerStr == "1" || lowerStr == "yes" || lowerStr == "on") {
            return ConfigValue(true);
        }
        if (lowerStr == "false" || lowerStr == "0" || lowerStr == "no" || lowerStr == "off") {
            return ConfigValue(false);
        }
        throw std::runtime_error("Invalid boolean value: " + str);
    }
    if (std::holds_alternative<int>(defaultValue)) {
        try {
            return ConfigValue(std::stoi(str));
        } catch (const std::exception&) {
            throw std::runtime_error("Invalid integer value: " + str);
        }
    }
    if (std::holds_alternative<double>(defaultValue)) {
        try {
            return ConfigValue(std::stod(str));
        } catch (const std::exception&) {
            throw std::runtime_error("Invalid double value: " + str);
        }
    }
    if (std::holds_alternative<std::vector<std::string>>(defaultValue)) {
        std::vector<std::string> result;
        std::stringstream ss(str);
        std::string item;
        while (std::getline(ss, item, ',')) {
            result.push_back(item);
        }
        return ConfigValue(result);
    }
    
    // Default to string
    return ConfigValue(str);
}
