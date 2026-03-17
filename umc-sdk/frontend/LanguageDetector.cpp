#include "LanguageDetector.h"
#include <fstream>
#include <algorithm>
#include <cctype>
#include <regex>
#include <iostream>
#include <sstream>

Language LanguageDetector::detectLanguage(const std::string& filename) {
    Language lang = detectByExtension(filename);
    if (lang != Language::UNKNOWN) {
        return lang;
    }

    // If extension detection fails, try content detection
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return detectByContent(content);
    }

    return Language::UNKNOWN;
}

Language LanguageDetector::detectByContent(const std::string& content) {
    Language lang = detectByShebang(content);
    if (lang != Language::UNKNOWN) {
        return lang;
    }

    return detectBySyntax(content);
}

std::string LanguageDetector::languageToString(Language lang) {
    switch (lang) {
        case Language::C: return "C";
        case Language::CPP: return "C++";
        case Language::JAVA: return "Java";
        case Language::PYTHON: return "Python";
        default: return "Unknown";
    }
}

Language LanguageDetector::detectByExtension(const std::string& filename) {
    size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) {
        return Language::UNKNOWN;
    }

    std::string extension = filename.substr(dotPos + 1);
    std::transform(extension.begin(), extension.end(), extension.begin(), 
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

    if (extension == "c") return Language::C;
    if (extension == "cpp" || extension == "cxx" || extension == "cc") return Language::CPP;
    if (extension == "java") return Language::JAVA;
    if (extension == "py") return Language::PYTHON;

    return Language::UNKNOWN;
}

Language LanguageDetector::detectByShebang(const std::string& content) {
    if (content.empty()) return Language::UNKNOWN;

    if (content.substr(0, 2) == "#!") {
        std::string firstLine = content.substr(0, content.find('\n'));
        if (firstLine.find("python") != std::string::npos) {
            return Language::PYTHON;
        }
    }

    return Language::UNKNOWN;
}

Language LanguageDetector::detectBySyntax(const std::string& content) {
    if (content.empty()) return Language::UNKNOWN;

    // Enhanced syntax detection with regex patterns
    std::regex c_cpp_patterns[] = {
        std::regex(R"(\bint\s+\w+\s*\()"),           // Function definitions
        std::regex(R"(\b#include\s+[<"])"),          // Include statements
        std::regex(R"(\bstd::\w+)"),                 // C++ standard library
        std::regex(R"(\bclass\s+\w+)"),              // Class definitions
        std::regex(R"(\bpublic\s*:\s*)"),            // Access specifiers
        std::regex(R"(\btemplate\s*<)"),             // Templates
        std::regex(R"(\bprintf\s*\()"),              // Printf calls
        std::regex(R"(\bmalloc\s*\()"),              // Malloc calls
    };

    std::regex java_patterns[] = {
        std::regex(R"(\bpublic\s+class\s+\w+)"),     // Public class
        std::regex(R"(\bimport\s+\w+)"),             // Import statements
        std::regex(R"(\bSystem\.out\.print)"),       // System.out.print
        std::regex(R"(\bpublic\s+static\s+void\s+main)"), // Main method
        std::regex(R"(\bextends\s+\w+)"),            // Inheritance
        std::regex(R"(\bimplements\s+\w+)"),         // Interfaces
    };

    std::regex python_patterns[] = {
        std::regex(R"(\bdef\s+\w+\s*\()"),           // Function definitions
        std::regex(R"(\bimport\s+\w+)"),             // Import statements
        std::regex(R"(\bfrom\s+\w+\s+import)"),      // From imports
        std::regex(R"(\bprint\s*\()"),               // Print statements
        std::regex(R"(\bif\s+__name__\s*==\s*['"]__main__['"])"), // Main guard
        std::regex(R"(\.py\b)"),                     // .py extension in strings
    };

    int c_cpp_score = 0;
    int java_score = 0;
    int python_score = 0;

    // Count matches for each language
    for (const auto& pattern : c_cpp_patterns) {
        if (std::regex_search(content, pattern)) {
            c_cpp_score++;
        }
    }

    for (const auto& pattern : java_patterns) {
        if (std::regex_search(content, pattern)) {
            java_score++;
        }
    }

    for (const auto& pattern : python_patterns) {
        if (std::regex_search(content, pattern)) {
            python_score++;
        }
    }

    // Also check for basic syntax indicators
    size_t brace_count = 0;
    size_t semicolon_count = 0;
    size_t colon_count = 0;
    size_t indent_spaces = 0;

    for (char c : content) {
        if (c == '{') brace_count++;
        if (c == '}') brace_count++;
        if (c == ';') semicolon_count++;
        if (c == ':') colon_count++;
    }

    // Count indentation patterns
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
        if (line.find("    ") == 0 || line.find("\t") == 0) {
            indent_spaces++;
        }
    }

    // Decision logic with scores
    if (c_cpp_score > 0 || (brace_count > 2 && semicolon_count > 0)) {
        if (java_score > c_cpp_score || colon_count > brace_count / 2) {
            return Language::JAVA;
        }
        return content.find("std::") != std::string::npos ? Language::CPP : Language::C;
    }

    if (python_score > 0 || indent_spaces > 3) {
        return Language::PYTHON;
    }

    if (java_score > 0) {
        return Language::JAVA;
    }

    return Language::UNKNOWN;
}
