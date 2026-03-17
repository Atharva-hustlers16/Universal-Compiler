#include "PerformanceBenchmark.h"
#include <iostream>
#include <algorithm>
#include <numeric>
#include <ctime>
#include <sstream>

#ifdef _WIN32
#include <windows.h>
#include <psapi.h>
#pragma comment(lib, "psapi.lib")
#else
#include <sys/resource.h>
#include <unistd.h>
#endif

PerformanceBenchmark& PerformanceBenchmark::getInstance() {
    static PerformanceBenchmark instance;
    return instance;
}

void PerformanceBenchmark::registerBenchmark(const std::string& suite, const std::string& name,
                                            std::function<BenchmarkResult()> benchmarkFunc) {
    BenchmarkFunction benchmark;
    benchmark.name = name;
    benchmark.func = benchmarkFunc;
    benchmark.isSimple = false;
    benchmarkSuites_[suite].push_back(benchmark);
}

void PerformanceBenchmark::registerSimpleBenchmark(const std::string& suite, const std::string& name,
                                                     std::function<void()> benchmarkFunc) {
    BenchmarkFunction benchmark;
    benchmark.name = name;
    benchmark.simpleFunc = benchmarkFunc;
    benchmark.isSimple = true;
    benchmarkSuites_[suite].push_back(benchmark);
}

bool PerformanceBenchmark::runAllBenchmarks() {
    std::cout << "\n=== Running All Benchmark Suites ===" << std::endl;
    
    bool allPassed = true;
    for (const auto& [suiteName, benchmarks] : benchmarkSuites_) {
        if (!runBenchmarkSuite(suiteName)) {
            allPassed = false;
        }
    }
    
    printResults();
    return allPassed;
}

bool PerformanceBenchmark::runBenchmarkSuite(const std::string& suiteName) {
    auto it = benchmarkSuites_.find(suiteName);
    if (it == benchmarkSuites_.end()) {
        std::cerr << "Benchmark suite '" << suiteName << "' not found!" << std::endl;
        return false;
    }
    
    std::cout << "\n--- Running Benchmark Suite: " << suiteName << " ---" << std::endl;
    
    // Initialize suite results
    BenchmarkSuite& suite = results_[suiteName];
    suite.name = suiteName;
    suite.results.clear();
    suite.totalDurationMs = 0.0;
    suite.averageDurationMs = 0.0;
    suite.minDurationMs = std::numeric_limits<double>::max();
    suite.maxDurationMs = 0.0;
    
    for (const auto& benchmark : it->second) {
        BenchmarkResult result = runBenchmark(suiteName, benchmark);
        suite.results.push_back(result);
        suite.totalDurationMs += result.durationMs;
        suite.minDurationMs = std::min(suite.minDurationMs, result.durationMs);
        suite.maxDurationMs = std::max(suite.maxDurationMs, result.durationMs);
    }
    
    if (!suite.results.empty()) {
        suite.averageDurationMs = suite.totalDurationMs / suite.results.size();
    }
    
    printSuiteResults(suiteName);
    return true;
}

bool PerformanceBenchmark::runSingleBenchmark(const std::string& suiteName, const std::string& benchmarkName) {
    auto suiteIt = benchmarkSuites_.find(suiteName);
    if (suiteIt == benchmarkSuites_.end()) {
        std::cerr << "Benchmark suite '" << suiteName << "' not found!" << std::endl;
        return false;
    }
    
    auto benchmarkIt = std::find_if(suiteIt->second.begin(), suiteIt->second.end(),
                                   [&benchmarkName](const BenchmarkFunction& benchmark) {
                                       return benchmark.name == benchmarkName;
                                   });
    
    if (benchmarkIt == suiteIt->second.end()) {
        std::cerr << "Benchmark '" << benchmarkName << "' not found in suite '" << suiteName << "'!" << std::endl;
        return false;
    }
    
    std::cout << "\n--- Running Single Benchmark: " << suiteName << "::" << benchmarkName << " ---" << std::endl;
    
    BenchmarkResult result = runBenchmark(suiteName, *benchmarkIt);
    printBenchmarkResult(result);
    
    return true;
}

PerformanceBenchmark::BenchmarkResult PerformanceBenchmark::runBenchmark(const std::string& suiteName, 
                                                                         const BenchmarkFunction& benchmark) {
    BenchmarkResult result;
    result.name = benchmark.name;
    result.timestamp = getCurrentTimestamp();
    
    if (verbose_) {
        std::cout << "Running: " << benchmark.name << " ... ";
        std::cout.flush();
    }
    
    // Warmup phase
    for (int i = 0; i < warmupIterations_; ++i) {
        if (benchmark.isSimple) {
            benchmark.simpleFunc();
        } else {
            benchmark.func(); // Ignore warmup results
        }
    }
    
    // Memory tracking
    size_t startMemory = memoryProfiling_ ? getCurrentMemoryUsageBytes() : 0;
    size_t peakMemory = startMemory;
    
    // Benchmark execution
    auto totalStartTime = std::chrono::high_resolution_clock::now();
    
    if (benchmark.isSimple) {
        // Simple benchmark - measure multiple iterations
        for (int i = 0; i < iterations_; ++i) {
            auto iterStartTime = std::chrono::high_resolution_clock::now();
            benchmark.simpleFunc();
            auto iterEndTime = std::chrono::high_resolution_clock::now();
            
            if (memoryProfiling_) {
                size_t currentMemory = getCurrentMemoryUsageBytes();
                peakMemory = std::max(peakMemory, currentMemory);
            }
        }
        
        auto totalEndTime = std::chrono::high_resolution_clock::now();
        auto totalDuration = std::chrono::duration_cast<std::chrono::microseconds>(totalEndTime - totalStartTime);
        result.durationMs = totalDuration.count() / 1000.0 / iterations_;
        
        result.operationsPerSecond = iterations_ * 1000.0 / totalDuration.count();
    } else {
        // Advanced benchmark - let the function handle its own timing
        result = benchmark.func();
        
        if (memoryProfiling_) {
            size_t currentMemory = getCurrentMemoryUsageBytes();
            peakMemory = std::max(peakMemory, currentMemory);
        }
    }
    
    result.memoryUsageMB = memoryProfiling_ ? (peakMemory - startMemory) / (1024.0 * 1024.0) : 0.0;
    
    if (verbose_) {
        std::cout << std::fixed << std::setprecision(2) << result.durationMs << "ms";
        if (memoryProfiling_) {
            std::cout << ", " << std::setprecision(1) << result.memoryUsageMB << "MB";
        }
        std::cout << std::endl;
    }
    
    updateSuiteStatistics(suiteName, result);
    return result;
}

void PerformanceBenchmark::printResults() const {
    std::cout << "\n=== Benchmark Results Summary ===" << std::endl;
    
    double grandTotalDuration = 0.0;
    int totalBenchmarks = 0;
    
    for (const auto& [suiteName, suite] : results_) {
        std::cout << suiteName << ": " << suite.results.size() << " benchmarks, ";
        std::cout << std::fixed << std::setprecision(2) << suite.totalDurationMs << "ms total, ";
        std::cout << std::setprecision(2) << suite.averageDurationMs << "ms avg" << std::endl;
        
        grandTotalDuration += suite.totalDurationMs;
        totalBenchmarks += suite.results.size();
    }
    
    std::cout << "\nOverall: " << totalBenchmarks << " benchmarks, ";
    std::cout << std::fixed << std::setprecision(2) << grandTotalDurationMs << "ms total" << std::endl;
    
    // Find fastest and slowest benchmarks
    double fastestMs = std::numeric_limits<double>::max();
    double slowestMs = 0.0;
    std::string fastestName, slowestName;
    
    for (const auto& [suiteName, suite] : results_) {
        for (const auto& result : suite.results) {
            if (result.durationMs < fastestMs) {
                fastestMs = result.durationMs;
                fastestName = suiteName + "::" + result.name;
            }
            if (result.durationMs > slowestMs) {
                slowestMs = result.durationMs;
                slowestName = suiteName + "::" + result.name;
            }
        }
    }
    
    std::cout << "Fastest: " << fastestName << " (" << std::fixed << std::setprecision(2) << fastestMs << "ms)" << std::endl;
    std::cout << "Slowest: " << slowestName << " (" << std::fixed << std::setprecision(2) << slowestMs << "ms)" << std::endl;
    std::cout << std::endl;
}

void PerformanceBenchmark::printSuiteResults(const std::string& suiteName) const {
    auto it = results_.find(suiteName);
    if (it == results_.end()) {
        return;
    }
    
    const BenchmarkSuite& suite = it->second;
    
    std::cout << "Suite Results: " << suite.results.size() << " benchmarks, ";
    std::cout << std::fixed << std::setprecision(2) << suite.totalDurationMs << "ms total, ";
    std::cout << std::setprecision(2) << suite.averageDurationMs << "ms avg, ";
    std::cout << std::setprecision(2) << suite.minDurationMs << "ms min, ";
    std::cout << std::setprecision(2) << suite.maxDurationMs << "ms max" << std::endl;
}

void PerformanceBenchmark::exportResults(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file for writing: " << filename << std::endl;
        return;
    }
    
    file << "UMC-SDK Performance Benchmark Results\n";
    file << "=====================================\n\n";
    
    for (const auto& [suiteName, suite] : results_) {
        file << "Benchmark Suite: " << suiteName << "\n";
        file << "Total Benchmarks: " << suite.results.size() << "\n";
        file << "Total Duration: " << std::fixed << std::setprecision(2) << suite.totalDurationMs << "ms\n";
        file << "Average Duration: " << std::setprecision(2) << suite.averageDurationMs << "ms\n";
        file << "Min Duration: " << std::setprecision(2) << suite.minDurationMs << "ms\n";
        file << "Max Duration: " << std::setprecision(2) << suite.maxDurationMs << "ms\n\n";
        
        file << "Benchmark Details:\n";
        for (const auto& result : suite.results) {
            file << "  " << result.name << "\n";
            file << "    Duration: " << std::fixed << std::setprecision(2) << result.durationMs << "ms\n";
            file << "    Memory Usage: " << std::setprecision(1) << result.memoryUsageMB << "MB\n";
            file << "    Operations/sec: " << std::setprecision(0) << result.operationsPerSecond << "\n";
            file << "    Timestamp: " << result.timestamp << "\n";
            
            if (!result.customMetrics.empty()) {
                file << "    Custom Metrics:\n";
                for (const auto& [metric, value] : result.customMetrics) {
                    file << "      " << metric << ": " << std::fixed << std::setprecision(2) << value << "\n";
                }
            }
            file << "\n";
        }
        file << "\n";
    }
    
    file.close();
    std::cout << "Results exported to: " << filename << std::endl;
}

void PerformanceBenchmark::exportResultsCSV(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open CSV file for writing: " << filename << std::endl;
        return;
    }
    
    // CSV header
    file << "Suite,Benchmark,Duration (ms),Memory Usage (MB),Operations/sec,Timestamp\n";
    
    for (const auto& [suiteName, suite] : results_) {
        for (const auto& result : suite.results) {
            file << suiteName << "," << result.name << ","
                 << std::fixed << std::setprecision(2) << result.durationMs << ","
                 << std::setprecision(1) << result.memoryUsageMB << ","
                 << std::setprecision(0) << result.operationsPerSecond << ","
                 << result.timestamp << "\n";
        }
    }
    
    file.close();
    std::cout << "CSV results exported to: " << filename << std::endl;
}

void PerformanceBenchmark::generateReport(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open report file for writing: " << filename << std::endl;
        return;
    }
    
    file << "# UMC-SDK Performance Report\n\n";
    file << "## Summary\n\n";
    
    int totalBenchmarks = getTotalBenchmarks();
    double totalDuration = getTotalDuration();
    
    file << "- **Total Benchmarks**: " << totalBenchmarks << "\n";
    file << "- **Total Duration**: " << std::fixed << std::setprecision(2) << totalDuration << "ms\n";
    file << "- **Average Duration**: " << std::setprecision(2) << (totalDuration / totalBenchmarks) << "ms\n\n";
    
    file << "## Benchmark Suites\n\n";
    
    for (const auto& [suiteName, suite] : results_) {
        file << "### " << suiteName << "\n\n";
        file << "| Benchmark | Duration (ms) | Memory (MB) | Ops/sec |\n";
        file << "|-----------|---------------|------------|---------|\n";
        
        for (const auto& result : suite.results) {
            file << "| " << result.name << " | "
                 << std::fixed << std::setprecision(2) << result.durationMs << " | "
                 << std::setprecision(1) << result.memoryUsageMB << " | "
                 << std::setprecision(0) << result.operationsPerSecond << " |\n";
        }
        
        file << "\n**Statistics**: " << suite.results.size() << " benchmarks, "
             << std::fixed << std::setprecision(2) << suite.totalDurationMs << "ms total, "
             << std::setprecision(2) << suite.averageDurationMs << "ms average\n\n";
    }
    
    file.close();
    std::cout << "Performance report generated: " << filename << std::endl;
}

void PerformanceBenchmark::compareWithBaseline(const std::string& baselineFile) const {
    std::ifstream file(baselineFile);
    if (!file.is_open()) {
        std::cerr << "Could not open baseline file: " << baselineFile << std::endl;
        return;
    }
    
    std::cout << "\n=== Performance Comparison with Baseline ===" << std::endl;
    
    // Simple comparison - in a real implementation, this would be more sophisticated
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("Total Duration:") != std::string::npos) {
            std::cout << "Baseline: " << line << std::endl;
            break;
        }
    }
    
    file.close();
    
    double currentTotal = getTotalDuration();
    std::cout << "Current: Total Duration: " << std::fixed << std::setprecision(2) << currentTotal << "ms" << std::endl;
}

void PerformanceBenchmark::detectPerformanceRegressions(double thresholdPercent) const {
    std::cout << "\n=== Performance Regression Detection ===" << std::endl;
    std::cout << "Threshold: " << thresholdPercent << "%" << std::endl;
    
    // This is a simplified implementation
    // In a real system, you would compare with historical data
    for (const auto& [suiteName, suite] : results_) {
        for (const auto& result : suite.results) {
            if (result.durationMs > 1000.0) { // Simple threshold
                std::cout << "WARNING: " << suiteName << "::" << result.name 
                         << " took " << std::fixed << std::setprecision(2) << result.durationMs 
                         << "ms (possible regression)" << std::endl;
            }
        }
    }
}

int PerformanceBenchmark::getTotalBenchmarks() const {
    int total = 0;
    for (const auto& [suiteName, suite] : results_) {
        total += suite.results.size();
    }
    return total;
}

double PerformanceBenchmark::getTotalDuration() const {
    double total = 0.0;
    for (const auto& [suiteName, suite] : results_) {
        total += suite.totalDurationMs;
    }
    return total;
}

std::vector<std::string> PerformanceBenchmark::getSuiteNames() const {
    std::vector<std::string> names;
    for (const auto& [suiteName, suite] : results_) {
        names.push_back(suiteName);
    }
    return names;
}

std::string PerformanceBenchmark::getCurrentTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    ss << "." << std::setfill('0') << std::setw(3) << ms.count();
    
    return ss.str();
}

void PerformanceBenchmark::updateSuiteStatistics(const std::string& suiteName, const BenchmarkResult& result) {
    // Statistics are updated in runBenchmarkSuite
}

void PerformanceBenchmark::printBenchmarkResult(const BenchmarkResult& result) const {
    std::cout << result.name << ": " << std::fixed << std::setprecision(2) << result.durationMs << "ms";
    if (memoryProfiling_) {
        std::cout << ", " << std::setprecision(1) << result.memoryUsageMB << "MB";
    }
    if (result.operationsPerSecond > 0) {
        std::cout << ", " << std::setprecision(0) << result.operationsPerSecond << " ops/sec";
    }
    std::cout << std::endl;
}

size_t PerformanceBenchmark::getCurrentMemoryUsageBytes() const {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
#else
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss * 1024; // Convert KB to bytes
    }
#endif
    return 0;
}

// BenchmarkTimer implementation
BenchmarkTimer::BenchmarkTimer(const std::string& name) : name_(name) {
    startTime_ = std::chrono::high_resolution_clock::now();
}

BenchmarkTimer::~BenchmarkTimer() {
    if (!stopped_) {
        stop();
    }
}

double BenchmarkTimer::getElapsedMs() const {
    auto endTime = stopped_ ? endTime_ : std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime_);
    return duration.count() / 1000.0;
}

void BenchmarkTimer::stop() {
    if (!stopped_) {
        endTime_ = std::chrono::high_resolution_clock::now();
        stopped_ = true;
        
        std::cout << name_ << ": " << std::fixed << std::setprecision(2) << getElapsedMs() << "ms" << std::endl;
    }
}

// MemoryProfiler implementation
MemoryProfiler::MemoryProfiler(const std::string& name) : name_(name) {
#ifdef _WIN32
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        startMemory_ = pmc.WorkingSetSize;
    }
#else
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        startMemory_ = usage.ru_maxrss * 1024;
    }
#endif
    peakMemory_ = startMemory_;
}

MemoryProfiler::~MemoryProfiler() {
    double peakMB = peakMemory_ / (1024.0 * 1024.0);
    double deltaMB = (peakMemory_ - startMemory_) / (1024.0 * 1024.0);
    
    std::cout << name_ << " memory: peak " << std::fixed << std::setprecision(1) << peakMB 
              << "MB, delta " << std::setprecision(1) << deltaMB << "MB" << std::endl;
}

double MemoryProfiler::getPeakMemoryMB() const {
    return peakMemory_ / (1024.0 * 1024.0);
}
