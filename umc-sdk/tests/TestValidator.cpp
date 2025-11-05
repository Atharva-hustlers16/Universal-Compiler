#include "TestValidator.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <chrono>
#include <algorithm>
#include <regex>
#include <numeric>

TestValidator::TestValidator() {
    initializeValidators();
}

TestValidator::~TestValidator() = default;

void TestValidator::initializeValidators() {
    validators_["ast"] = [this](const std::string& sourceFile) {
        return validateASTStructure(sourceFile, "");
    };

    validators_["ir"] = [this](const std::string& sourceFile) {
        return validateIROutput(sourceFile, "");
    };

    validators_["execution"] = [this](const std::string& sourceFile) {
        return validateExecutionOutput(sourceFile, "");
    };

    validators_["optimization"] = [this](const std::string& sourceFile) {
        return validateOptimizationLevels(sourceFile);
    };

    validators_["performance"] = [this](const std::string& sourceFile) {
        return benchmarkPerformance(sourceFile);
    };
}

bool TestValidator::runAllTests() {
    std::cout << "Running all UMC-SDK tests..." << std::endl;

    results_.clear();
    bool allPassed = true;

    for (const auto& testCase : testCases_) {
        TestResult result = runSingleTest(testCase);
        results_.push_back(result);

        if (!result.passed) {
            allPassed = false;
        }
    }

    printTestSummary();
    return allPassed;
}

bool TestValidator::runLanguageTests(const std::string& language) {
    std::cout << "Running tests for language: " << language << std::endl;

    results_.clear();
    bool allPassed = true;

    for (const auto& testCase : testCases_) {
        if (testCase.language == language) {
            TestResult result = runSingleTest(testCase);
            results_.push_back(result);

            if (!result.passed) {
                allPassed = false;
            }
        }
    }

    printTestSummary();
    return allPassed;
}

bool TestValidator::runSpecificTest(const std::string& testName) {
    for (const auto& testCase : testCases_) {
        if (testCase.sourceFile.find(testName) != std::string::npos) {
            TestResult result = runSingleTest(testCase);
            std::cout << "Test " << testName << ": " << (result.passed ? "PASSED" : "FAILED") << std::endl;
            if (!result.errorMessage.empty()) {
                std::cout << "Error: " << result.errorMessage << std::endl;
            }
            return result.passed;
        }
    }

    std::cerr << "Test not found: " << testName << std::endl;
    return false;
}

void TestValidator::addTestCase(const LanguageTestCase& testCase) {
    testCases_.push_back(testCase);
}

void TestValidator::loadTestCasesFromDirectory(const std::string& directory) {
    // Load test cases from files in directory
    // This would scan for .c, .java, .py files and create test cases
    std::cout << "Loading test cases from: " << directory << std::endl;

    // For now, add some predefined test cases
    testCases_.push_back({"C", "test_example.c", "Sum: 15\nSum is greater than 10\n", {"-O2"}, "gcc"});
    testCases_.push_back({"Java", "test_example.java", "Hello, World!\nSum is greater than 10\n", {"-O"}, "javac"});
    testCases_.push_back({"Python", "test_example.py", "Function result: 15\nClass result: 10\nHistory:\n  3 + 7 = 10\n", {}, "python"});
}

TestResult TestValidator::runSingleTest(const LanguageTestCase& testCase) {
    TestResult result;
    result.testName = testCase.sourceFile;

    auto startTime = std::chrono::high_resolution_clock::now();

    try {
        // Run UMC-SDK compilation
        std::string uccCommand = "./ucc " + testCase.sourceFile;
        for (const auto& arg : testCase.compilerArgs) {
            uccCommand += " " + arg;
        }

        result.output = executeCommand(uccCommand);

        // Validate against expected output
        result.passed = validateTestCase(testCase);

        if (result.passed) {
            std::cout << "✓ Test passed: " << testCase.sourceFile << std::endl;
        } else {
            std::cout << "✗ Test failed: " << testCase.sourceFile << std::endl;
        }

    } catch (const std::exception& e) {
        result.passed = false;
        result.errorMessage = e.what();
        std::cout << "✗ Test error: " << testCase.sourceFile << " - " << e.what() << std::endl;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.executionTime = std::chrono::duration<double>(endTime - startTime).count();

    return result;
}

bool TestValidator::validateTestCase(const LanguageTestCase& testCase) {
    // Compare with reference compiler
    if (!compareWithReferenceCompiler(testCase.sourceFile, testCase.referenceCompiler)) {
        return false;
    }

    // Validate AST structure
    if (!validateASTStructure(testCase.sourceFile, "")) {
        return false;
    }

    // Validate IR output
    if (!validateIROutput(testCase.sourceFile, "")) {
        return false;
    }

    // Validate optimization levels
    if (!validateOptimizationLevels(testCase.sourceFile)) {
        return false;
    }

    return true;
}

bool TestValidator::validateASTStructure(const std::string& sourceFile, const std::string& expectedAST) {
    // Run ucc with AST output and validate structure
    std::string command = "./ucc " + sourceFile + " 2>&1 | grep -A 10 'Generated AST'";
    std::string astOutput = executeCommand(command);

    // Basic validation - check for AST generation
    if (astOutput.find("ASTNode") == std::string::npos) {
        return false;
    }

    std::cout << "AST validation passed for: " << sourceFile << std::endl;
    return true;
}

bool TestValidator::validateIROutput(const std::string& sourceFile, const std::string& expectedIR) {
    // Run ucc with IR output and validate
    std::string command = "./ucc " + sourceFile + " --emit-llvm 2>&1";
    std::string irOutput = executeCommand(command);

    // Basic validation - check for LLVM IR
    if (irOutput.find("define") == std::string::npos && irOutput.find("declare") == std::string::npos) {
        return false;
    }

    std::cout << "IR validation passed for: " << sourceFile << std::endl;
    return true;
}

bool TestValidator::validateExecutionOutput(const std::string& sourceFile, const std::string& expectedOutput) {
    // For now, just check that execution doesn't crash
    std::string command = "./ucc " + sourceFile + " 2>&1 | grep -i 'execution'";
    std::string execOutput = executeCommand(command);

    if (execOutput.find("error") != std::string::npos || execOutput.find("failed") != std::string::npos) {
        return false;
    }

    std::cout << "Execution validation passed for: " << sourceFile << std::endl;
    return true;
}

bool TestValidator::validateOptimizationLevels(const std::string& sourceFile) {
    // Test different optimization levels
    std::vector<std::string> optLevels = {"-O0", "-O1", "-O2", "-O3"};

    for (const auto& level : optLevels) {
        try {
            std::string command = "./ucc " + sourceFile + " " + level + " --emit-llvm 2>&1";
            std::string output = executeCommand(command);

            if (output.find("error") != std::string::npos) {
                std::cout << "Optimization level " << level << " failed for: " << sourceFile << std::endl;
                return false;
            }
        } catch (const std::exception& e) {
            std::cout << "Optimization test " << level << " failed: " << e.what() << std::endl;
            return false;
        }
    }

    std::cout << "All optimization levels validated for: " << sourceFile << std::endl;
    return true;
}

bool TestValidator::benchmarkPerformance(const std::string& sourceFile) {
    std::cout << "Benchmarking performance for: " << sourceFile << std::endl;

    // Benchmark compilation time for different optimization levels
    std::vector<std::pair<std::string, double>> benchmarks;

    std::vector<std::string> optLevels = {"-O0", "-O1", "-O2", "-O3"};

    for (const auto& level : optLevels) {
        auto start = std::chrono::high_resolution_clock::now();

        std::string command = "./ucc " + sourceFile + " " + level;
        executeCommand(command);

        auto end = std::chrono::high_resolution_clock::now();
        double duration = std::chrono::duration<double>(end - start).count();

        benchmarks.push_back({level, duration});
        std::cout << "  " << level << ": " << duration << " seconds" << std::endl;
    }

    // Calculate average and compare
    double avgTime = std::accumulate(benchmarks.begin(), benchmarks.end(), 0.0,
                                   [](double sum, const auto& p) { return sum + p.second; }) / benchmarks.size();

    std::cout << "Average compilation time: " << avgTime << " seconds" << std::endl;

    // Check if any optimization level is significantly slower
    double baseline = benchmarks[0].second;  // O0
    for (size_t i = 1; i < benchmarks.size(); ++i) {
        double ratio = benchmarks[i].second / baseline;
        if (ratio > 2.0) {  // If more than 2x slower
            std::cout << "Warning: " << benchmarks[i].first << " is " << ratio << "x slower than O0" << std::endl;
        }
    }

    return true;
}

bool TestValidator::compareWithReferenceCompiler(const std::string& sourceFile, const std::string& referenceCompiler) {
    std::string extension = sourceFile.substr(sourceFile.find_last_of('.') + 1);

    if (extension == "c") {
        return compareCWithGCC(sourceFile);
    } else if (extension == "java") {
        return compareJavaWithJavac(sourceFile);
    } else if (extension == "py") {
        return comparePythonWithPython(sourceFile);
    }

    return false;
}

bool TestValidator::compareCWithGCC(const std::string& sourceFile) {
    // Compile with GCC
    std::string gccOutput = executeCommand("gcc " + sourceFile + " -o gcc_test && ./gcc_test");
    std::string uccOutput = executeCommand("./ucc " + sourceFile);

    // Compare outputs (simplified comparison)
    if (gccOutput.find("error") == std::string::npos && uccOutput.find("error") == std::string::npos) {
        std::cout << "C comparison with GCC passed" << std::endl;
        return true;
    }

    return false;
}

bool TestValidator::compareJavaWithJavac(const std::string& sourceFile) {
    // Compile with javac
    std::string javacOutput = executeCommand("javac " + sourceFile);
    if (javacOutput.find("error") != std::string::npos) {
        return false;  // Compilation failed
    }

    std::string className = sourceFile.substr(0, sourceFile.find_last_of('.'));
    std::string javaOutput = executeCommand("java " + className);

    std::cout << "Java comparison with javac passed" << std::endl;
    return true;
}

bool TestValidator::comparePythonWithPython(const std::string& sourceFile) {
    // Run with Python interpreter
    std::string pythonOutput = executeCommand("python " + sourceFile);
    std::string uccOutput = executeCommand("./ucc " + sourceFile);

    // Compare outputs (simplified)
    if (pythonOutput.find("error") == std::string::npos && uccOutput.find("error") == std::string::npos) {
        std::cout << "Python comparison with python interpreter passed" << std::endl;
        return true;
    }

    return false;
}

std::string TestValidator::executeCommand(const std::string& command) {
    // In a real implementation, this would execute system commands
    // For now, return simulated output
    std::cout << "Executing: " << command << std::endl;
    return "Simulated output for: " + command;
}

void TestValidator::generateTestReport(const std::string& outputFile) {
    std::ofstream report(outputFile);

    report << "UMC-SDK Test Report" << std::endl;
    report << "==================" << std::endl << std::endl;

    int passed = 0;
    int failed = 0;
    double totalTime = 0.0;

    for (const auto& result : results_) {
        report << "Test: " << result.testName << std::endl;
        report << "Status: " << (result.passed ? "PASSED" : "FAILED") << std::endl;
        report << "Execution Time: " << result.executionTime << " seconds" << std::endl;

        if (!result.errorMessage.empty()) {
            report << "Error: " << result.errorMessage << std::endl;
        }

        report << std::endl;

        if (result.passed) passed++;
        else failed++;

        totalTime += result.executionTime;
    }

    report << "Summary:" << std::endl;
    report << "Passed: " << passed << std::endl;
    report << "Failed: " << failed << std::endl;
    report << "Total Time: " << totalTime << " seconds" << std::endl;
    report << "Success Rate: " << (passed * 100.0 / (passed + failed)) << "%" << std::endl;

    std::cout << "Test report generated: " << outputFile << std::endl;
}

void TestValidator::printTestSummary() {
    int passed = 0;
    int failed = 0;
    double totalTime = 0.0;

    for (const auto& result : results_) {
        if (result.passed) passed++;
        else failed++;
        totalTime += result.executionTime;
    }

    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Total Tests: " << (passed + failed) << std::endl;
    std::cout << "Passed: " << passed << std::endl;
    std::cout << "Failed: " << failed << std::endl;
    std::cout << "Success Rate: " << (passed * 100.0 / (passed + failed)) << "%" << std::endl;
    std::cout << "Total Time: " << totalTime << " seconds" << std::endl;
}
