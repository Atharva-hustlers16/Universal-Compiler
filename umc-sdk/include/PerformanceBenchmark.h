#ifndef PERFORMANCE_BENCHMARKER_H
#define PERFORMANCE_BENCHMARKER_H

#include <string>
#include <vector>
#include <chrono>
#include <functional>
#include <memory>
#include <map>
#include <fstream>
#include <iomanip>

class PerformanceBenchmark {
public:
    struct BenchmarkResult {
        std::string name;
        double durationMs;
        double memoryUsageMB;
        size_t operationsPerSecond;
        std::map<std::string, double> customMetrics;
        std::string timestamp;
    };
    
    struct BenchmarkSuite {
        std::string name;
        std::vector<BenchmarkResult> results;
        double totalDurationMs = 0.0;
        double averageDurationMs = 0.0;
        double minDurationMs = std::numeric_limits<double>::max();
        double maxDurationMs = 0.0;
    };
    
    static PerformanceBenchmark& getInstance();
    
    // Benchmark registration and execution
    void registerBenchmark(const std::string& suite, const std::string& name,
                          std::function<BenchmarkResult()> benchmarkFunc);
    void registerSimpleBenchmark(const std::string& suite, const std::string& name,
                                 std::function<void()> benchmarkFunc);
    
    // Execution
    bool runAllBenchmarks();
    bool runBenchmarkSuite(const std::string& suiteName);
    bool runSingleBenchmark(const std::string& suiteName, const std::string& benchmarkName);
    
    // Results and reporting
    void printResults() const;
    void printSuiteResults(const std::string& suiteName) const;
    void exportResults(const std::string& filename) const;
    void exportResultsCSV(const std::string& filename) const;
    void generateReport(const std::string& filename) const;
    
    // Comparison and analysis
    void compareWithBaseline(const std::string& baselineFile) const;
    void detectPerformanceRegressions(double thresholdPercent = 10.0) const;
    void generatePerformanceTrends() const;
    
    // Configuration
    void setIterations(int iterations) { iterations_ = iterations; }
    void setWarmupIterations(int warmupIterations) { warmupIterations_ = warmupIterations; }
    void setVerbose(bool verbose) { verbose_ = verbose; }
    void setMemoryProfiling(bool enabled) { memoryProfiling_ = enabled; }
    
    // Statistics
    int getTotalBenchmarks() const;
    double getTotalDuration() const;
    std::vector<std::string> getSuiteNames() const;

private:
    PerformanceBenchmark() : iterations_(100), warmupIterations_(10), verbose_(true), memoryProfiling_(true) {}
    
    struct BenchmarkFunction {
        std::string name;
        std::function<BenchmarkResult()> func;
        bool isSimple = false;
        std::function<void()> simpleFunc;
    };
    
    std::map<std::string, std::vector<BenchmarkFunction>> benchmarkSuites_;
    std::map<std::string, BenchmarkSuite> results_;
    
    int iterations_;
    int warmupIterations_;
    bool verbose_;
    bool memoryProfiling_;
    
    // Internal helpers
    BenchmarkResult runBenchmark(const std::string& suiteName, const BenchmarkFunction& benchmark);
    double getCurrentMemoryUsageMB() const;
    std::string getCurrentTimestamp() const;
    void updateSuiteStatistics(const std::string& suiteName, const BenchmarkResult& result);
    void printBenchmarkResult(const BenchmarkResult& result) const;
    
    // Memory profiling (platform-specific)
    size_t getCurrentMemoryUsageBytes() const;
};

// Convenience macros for benchmarking
#define BENCHMARK(suite, name) \
    BenchmarkResult benchmark_##suite##_##name(); \
    struct BenchmarkRegistrar_##suite##_##name { \
        BenchmarkRegistrar_##suite##_##name() { \
            PerformanceBenchmark::getInstance().registerBenchmark(#suite, #name, benchmark_##suite##_##name); \
        } \
    }; \
    static BenchmarkRegistrar_##suite##_##name registrar_##suite##_##name; \
    BenchmarkResult benchmark_##suite##_##name()

#define SIMPLE_BENCHMARK(suite, name) \
    void benchmark_simple_##suite##_##name(); \
    struct SimpleBenchmarkRegistrar_##suite##_##name { \
        SimpleBenchmarkRegistrar_##suite##_##name() { \
            PerformanceBenchmark::getInstance().registerSimpleBenchmark(#suite, #name, benchmark_simple_##suite##_##name); \
        } \
    }; \
    static SimpleBenchmarkRegistrar_##suite##_##name registrar_##suite##_##name; \
    void benchmark_simple_##suite##_##name()

// RAII Timer class for easy timing
class BenchmarkTimer {
public:
    BenchmarkTimer(const std::string& name);
    ~BenchmarkTimer();
    
    double getElapsedMs() const;
    void stop();
    
private:
    std::string name_;
    std::chrono::high_resolution_clock::time_point startTime_;
    std::chrono::high_resolution_clock::time_point endTime_;
    bool stopped_ = false;
};

// Scoped memory profiler
class MemoryProfiler {
public:
    MemoryProfiler(const std::string& name);
    ~MemoryProfiler();
    
    double getPeakMemoryMB() const;
    
private:
    std::string name_;
    size_t startMemory_;
    size_t peakMemory_;
};

#endif // PERFORMANCE_BENCHMARKER_H
