#include <iostream>
#include "TestValidator.h"
#include "LanguageDetector.h"
#include "FrontendBase.h"
#include "IRGenerator.h"

int main(int argc, char** argv) {
    std::cout << "UMC-SDK Comprehensive Test Suite" << std::endl;
    std::cout << "================================" << std::endl;

    TestValidator validator;

    // Load test cases
    validator.loadTestCasesFromDirectory(".");

    // Run all tests
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--language" && argc > 2) {
            std::string language = argv[2];
            validator.runLanguageTests(language);
        } else if (arg == "--test" && argc > 2) {
            std::string testName = argv[2];
            validator.runSpecificTest(testName);
        } else {
            std::cerr << "Usage: test_umc [--language <lang>] [--test <testname>]" << std::endl;
            return 1;
        }
    } else {
        // Run all tests
        bool allPassed = validator.runAllTests();

        // Generate report
        validator.generateTestReport("test_report.txt");

        return allPassed ? 0 : 1;
    }

    return 0;
}
