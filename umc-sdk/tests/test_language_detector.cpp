#include <iostream>
#include "LanguageDetector.h"
#include <cassert>

void testLanguageDetector() {
    // Test C detection
    Language c = LanguageDetector::detectLanguage("hello.c");
    assert(c == Language::C);

    // Test C++ detection
    Language cpp = LanguageDetector::detectLanguage("hello.cpp");
    assert(cpp == Language::CPP);

    // Test Java detection
    Language java = LanguageDetector::detectLanguage("Hello.java");
    assert(java == Language::JAVA);

    // Test Python detection
    Language python = LanguageDetector::detectLanguage("script.py");
    assert(python == Language::PYTHON);

    std::cout << "LanguageDetector tests passed!" << std::endl;
}
