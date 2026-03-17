#include "TestFramework.h"
#include <fstream>
#include <iomanip>
#include <algorithm>

TestFramework& TestFramework::getInstance() {
    static TestFramework instance;
    return instance;
}

void TestFramework::addTest(const std::string& suite, const std::string& name, 
                           std::function<bool()> testFunc) {
    TestCase test;
    test.name = name;
    test.testFunc = testFunc;
    test.isBenchmark = false;
    testSuites_[suite].push_back(test);
}

void TestFramework::addBenchmark(const std::string& suite, const std::string& name,
                                std::function<double()> benchmarkFunc) {
    TestCase test;
    test.name = name;
    test.testFunc = [benchmarkFunc]() -> bool { 
        benchmarkFunc(); 
        return true; 
    };
    test.isBenchmark = true;
    testSuites_[suite].push_back(test);
}

bool TestFramework::runAllTests() {
    std::cout << "\n=== Running All Test Suites ===" << std::endl;
    
    bool allPassed = true;
    for (const auto& [suiteName, tests] : testSuites_) {
        if (!runTestSuite(suiteName)) {
            allPassed = false;
            if (stopOnFirstFailure_) {
                break;
            }
        }
    }
    
    printResults();
    return allPassed;
}

bool TestFramework::runTestSuite(const std::string& suiteName) {
    auto it = testSuites_.find(suiteName);
    if (it == testSuites_.end()) {
        std::cerr << "Test suite '" << suiteName << "' not found!" << std::endl;
        return false;
    }
    
    std::cout << "\n--- Running Test Suite: " << suiteName << " ---" << std::endl;
    
    // Initialize suite results
    TestSuite& suite = results_[suiteName];
    suite.name = suiteName;
    suite.results.clear();
    suite.totalTests = 0;
    suite.passedTests = 0;
    suite.failedTests = 0;
    suite.totalDurationMs = 0.0;
    
    bool suitePassed = true;
    for (const auto& test : it->second) {
        TestResult result = runTest(suiteName, test);
        suite.results.push_back(result);
        suite.totalTests++;
        suite.totalDurationMs += result.durationMs;
        
        if (result.passed) {
            suite.passedTests++;
        } else {
            suite.failedTests++;
            suitePassed = false;
            if (stopOnFirstFailure_) {
                break;
            }
        }
    }
    
    printSuiteResults(suiteName);
    return suitePassed;
}

bool TestFramework::runSingleTest(const std::string& suiteName, const std::string& testName) {
    auto suiteIt = testSuites_.find(suiteName);
    if (suiteIt == testSuites_.end()) {
        std::cerr << "Test suite '" << suiteName << "' not found!" << std::endl;
        return false;
    }
    
    auto testIt = std::find_if(suiteIt->second.begin(), suiteIt->second.end(),
                               [&testName](const TestCase& test) {
                                   return test.name == testName;
                               });
    
    if (testIt == suiteIt->second.end()) {
        std::cerr << "Test '" << testName << "' not found in suite '" << suiteName << "'!" << std::endl;
        return false;
    }
    
    std::cout << "\n--- Running Single Test: " << suiteName << "::" << testName << " ---" << std::endl;
    
    TestResult result = runTest(suiteName, *testIt);
    
    std::cout << (result.passed ? "PASS" : "FAIL") << ": " << result.name;
    if (!result.passed) {
        std::cout << " - " << result.message;
    }
    std::cout << " (" << std::fixed << std::setprecision(2) << result.durationMs << "ms)" << std::endl;
    
    return result.passed;
}

TestFramework::TestResult TestFramework::runTest(const std::string& suiteName, const TestCase& test) {
    TestResult result;
    result.name = test.name;
    result.passed = false;
    result.message = "";
    
    auto startTime = std::chrono::high_resolution_clock::now();
    
    try {
        if (test.isBenchmark) {
            // Run benchmark multiple times for better accuracy
            const int iterations = 100;
            double totalTime = 0.0;
            
            for (int i = 0; i < iterations; ++i) {
                auto benchStart = std::chrono::high_resolution_clock::now();
                test.testFunc();
                auto benchEnd = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(benchEnd - benchStart);
                totalTime += duration.count() / 1000.0; // Convert to milliseconds
            }
            
            result.durationMs = totalTime / iterations;
            result.passed = true;
            result.message = "Benchmark: " + std::to_string(result.durationMs) + "ms avg";
        } else {
            result.passed = test.testFunc();
            auto endTime = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
            result.durationMs = duration.count() / 1000.0;
        }
    } catch (const std::exception& e) {
        result.passed = false;
        result.message = std::string("Exception: ") + e.what();
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        result.durationMs = duration.count() / 1000.0;
    } catch (...) {
        result.passed = false;
        result.message = "Unknown exception";
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        result.durationMs = duration.count() / 1000.0;
    }
    
    if (verbose_) {
        std::cout << (result.passed ? "PASS" : "FAIL") << ": " << result.name;
        if (!result.passed && !result.message.empty()) {
            std::cout << " - " << result.message;
        }
        std::cout << " (" << std::fixed << std::setprecision(2) << result.durationMs << "ms)" << std::endl;
    }
    
    return result;
}

void TestFramework::printResults() const {
    std::cout << "\n=== Test Results Summary ===" << std::endl;
    
    int totalTests = 0, totalPassed = 0, totalFailed = 0;
    double totalDuration = 0.0;
    
    for (const auto& [suiteName, suite] : results_) {
        totalTests += suite.totalTests;
        totalPassed += suite.passedTests;
        totalFailed += suite.failedTests;
        totalDuration += suite.totalDurationMs;
        
        std::cout << suiteName << ": " << suite.passedTests << "/" << suite.totalTests 
                  << " passed (" << std::fixed << std::setprecision(1) 
                  << (suite.totalTests > 0 ? (100.0 * suite.passedTests / suite.totalTests) : 0.0) 
                  << "%)" << std::endl;
    }
    
    std::cout << "\nOverall: " << totalPassed << "/" << totalTests 
              << " passed (" << std::fixed << std::setprecision(1) 
              << (totalTests > 0 ? (100.0 * totalPassed / totalTests) : 0.0) 
              << "%)" << std::endl;
    
    std::cout << "Total duration: " << std::fixed << std::setprecision(2) 
              << totalDuration << "ms" << std::endl;
    
    if (totalFailed > 0) {
        std::cout << "\nFailed tests:" << std::endl;
        for (const auto& [suiteName, suite] : results_) {
            for (const auto& result : suite.results) {
                if (!result.passed) {
                    std::cout << "  " << suiteName << "::" << result.name;
                    if (!result.message.empty()) {
                        std::cout << " - " << result.message;
                    }
                    std::cout << std::endl;
                }
            }
        }
    }
    
    std::cout << std::endl;
}

void TestFramework::printSuiteResults(const std::string& suiteName) const {
    auto it = results_.find(suiteName);
    if (it == results_.end()) {
        return;
    }
    
    const TestSuite& suite = it->second;
    
    std::cout << "Results: " << suite.passedTests << "/" << suite.totalTests 
              << " passed (" << std::fixed << std::setprecision(1) 
              << (suite.totalTests > 0 ? (100.0 * suite.passedTests / suite.totalTests) : 0.0) 
              << "%), " << std::fixed << std::setprecision(2) 
              << suite.totalDurationMs << "ms" << std::endl;
}

void TestFramework::exportResults(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }
    
    file << "UMC-SDK Test Results\n";
    file << "====================\n\n";
    
    for (const auto& [suiteName, suite] : results_) {
        file << "Test Suite: " << suiteName << "\n";
        file << "Total Tests: " << suite.totalTests << "\n";
        file << "Passed: " << suite.passedTests << "\n";
        file << "Failed: " << suite.failedTests << "\n";
        file << "Success Rate: " << std::fixed << std::setprecision(1) 
             << (suite.totalTests > 0 ? (100.0 * suite.passedTests / suite.totalTests) : 0.0) 
             << "%\n";
        file << "Duration: " << std::fixed << std::setprecision(2) << suite.totalDurationMs << "ms\n\n";
        
        file << "Test Details:\n";
        for (const auto& result : suite.results) {
            file << "  " << (result.passed ? "PASS" : "FAIL") << ": " << result.name;
            if (!result.passed && !result.message.empty()) {
                file << " - " << result.message;
            }
            file << " (" << std::fixed << std::setprecision(2) << result.durationMs << "ms)\n";
        }
        file << "\n";
    }
    
    file.close();
    std::cout << "Results exported to: " << filename << std::endl;
}

int TestFramework::getTotalTests() const {
    int total = 0;
    for (const auto& [suiteName, suite] : results_) {
        total += suite.totalTests;
    }
    return total;
}

int TestFramework::getPassedTests() const {
    int total = 0;
    for (const auto& [suiteName, suite] : results_) {
        total += suite.passedTests;
    }
    return total;
}

int TestFramework::getFailedTests() const {
    int total = 0;
    for (const auto& [suiteName, suite] : results_) {
        total += suite.failedTests;
    }
    return total;
}

double TestFramework::getTotalDuration() const {
    double total = 0.0;
    for (const auto& [suiteName, suite] : results_) {
        total += suite.totalDurationMs;
    }
    return total;
}
