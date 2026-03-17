#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <iostream>
#include <chrono>
#include <map>

// Test framework for UMC-SDK
class TestFramework {
public:
    struct TestResult {
        std::string name;
        bool passed;
        std::string message;
        double durationMs;
    };
    
    struct TestSuite {
        std::string name;
        std::vector<TestResult> results;
        int totalTests = 0;
        int passedTests = 0;
        int failedTests = 0;
        double totalDurationMs = 0.0;
    };
    
    static TestFramework& getInstance();
    
    // Test registration
    void addTest(const std::string& suite, const std::string& name, 
                 std::function<bool()> testFunc);
    void addBenchmark(const std::string& suite, const std::string& name,
                      std::function<double()> benchmarkFunc);
    
    // Test execution
    bool runAllTests();
    bool runTestSuite(const std::string& suiteName);
    bool runSingleTest(const std::string& suiteName, const std::string& testName);
    
    // Results and reporting
    void printResults() const;
    void printSuiteResults(const std::string& suiteName) const;
    void exportResults(const std::string& filename) const;
    
    // Statistics
    int getTotalTests() const;
    int getPassedTests() const;
    int getFailedTests() const;
    double getTotalDuration() const;
    
    // Configuration
    void setVerbose(bool verbose) { verbose_ = verbose; }
    void setStopOnFirstFailure(bool stop) { stopOnFirstFailure_ = stop; }

private:
    TestFramework() = default;
    
    struct TestCase {
        std::string name;
        std::function<bool()> testFunc;
        bool isBenchmark = false;
    };
    
    std::map<std::string, std::vector<TestCase>> testSuites_;
    std::map<std::string, TestSuite> results_;
    bool verbose_ = true;
    bool stopOnFirstFailure_ = false;
    
    TestResult runTest(const std::string& suiteName, const TestCase& test);
    void updateSuiteResults(const std::string& suiteName, const TestResult& result);
};

// Macros for easier test writing
#define TEST(suite, name) \
    bool test_##suite##_##name(); \
    struct TestRegistrar_##suite##_##name { \
        TestRegistrar_##suite##_##name() { \
            TestFramework::getInstance().addTest(#suite, #name, test_##suite##_##name); \
        } \
    }; \
    static TestRegistrar_##suite##_##name registrar_##suite##_##name; \
    bool test_##suite##_##name()

#define BENCHMARK(suite, name) \
    double benchmark_##suite##_##name(); \
    struct BenchmarkRegistrar_##suite##_##name { \
        BenchmarkRegistrar_##suite##_##name() { \
            TestFramework::getInstance().addBenchmark(#suite, #name, benchmark_##suite##_##name); \
        } \
    }; \
    static BenchmarkRegistrar_##suite##_##name registrar_##suite##_##name; \
    double benchmark_##suite##_##name()

// Assertion macros
#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            std::cerr << "ASSERT_TRUE failed: " << #condition << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            std::cerr << "ASSERT_FALSE failed: " << #condition << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            std::cerr << "ASSERT_EQ failed: expected " << (expected) << " but got " << (actual) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

#define ASSERT_NE(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            std::cerr << "ASSERT_NE failed: expected " << (expected) << " to not equal " << (actual) << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

#define ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != nullptr) { \
            std::cerr << "ASSERT_NULL failed: expected nullptr at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

#define ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == nullptr) { \
            std::cerr << "ASSERT_NOT_NULL failed: expected non-null pointer at " << __FILE__ << ":" << __LINE__ << std::endl; \
            return false; \
        } \
    } while(0)

#define FAIL(message) \
    do { \
        std::cerr << "FAIL: " << message << " at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return false; \
    } while(0)

#endif // TEST_FRAMEWORK_H
