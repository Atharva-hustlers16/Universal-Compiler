#ifndef TEST_VALIDATOR_H
#define TEST_VALIDATOR_H

#include <string>
#include <vector>
#include <map>
#include <functional>

struct TestResult {
    std::string testName;
    bool passed;
    std::string output;
    std::string expectedOutput;
    std::string errorMessage;
    double executionTime;
};

struct LanguageTestCase {
    std::string language;
    std::string sourceFile;
    std::string expectedOutput;
    std::vector<std::string> compilerArgs;
    std::string referenceCompiler;
};

class TestValidator {
public:
    TestValidator();
    ~TestValidator();

    // Test execution methods
    bool runAllTests();
    bool runLanguageTests(const std::string& language);
    bool runSpecificTest(const std::string& testName);

    // Test case management
    void addTestCase(const LanguageTestCase& testCase);
    void loadTestCasesFromDirectory(const std::string& directory);

    // Validation methods
    bool validateASTStructure(const std::string& sourceFile, const std::string& expectedAST);
    bool validateIROutput(const std::string& sourceFile, const std::string& expectedIR);
    bool validateExecutionOutput(const std::string& sourceFile, const std::string& expectedOutput);
    bool validateOptimizationLevels(const std::string& sourceFile);

    // Comparison with reference compilers
    bool compareWithReferenceCompiler(const std::string& sourceFile, const std::string& referenceCompiler);
    bool compareExecutionResults(const std::string& uccOutput, const std::string& referenceOutput);

    // Reporting
    void generateTestReport(const std::string& outputFile);
    void printTestSummary();

    // Utility methods
    std::string executeCommand(const std::string& command);
    bool compileWithReference(const std::string& sourceFile, const std::string& compiler, const std::string& outputFile);
    bool runExecutable(const std::string& executable, std::string& output);

private:
    std::vector<LanguageTestCase> testCases_;
    std::vector<TestResult> results_;
    std::map<std::string, std::function<bool(const std::string&)>> validators_;

    void initializeValidators();
    TestResult runSingleTest(const LanguageTestCase& testCase);
    bool validateTestCase(const LanguageTestCase& testCase);
    std::string extractFunctionFromIR(const std::string& irOutput, const std::string& functionName);

    // Reference compiler comparisons
    bool compareCWithGCC(const std::string& sourceFile);
    bool compareJavaWithJavac(const std::string& sourceFile);
    bool comparePythonWithPython(const std::string& sourceFile);

    // Performance benchmarking
    bool benchmarkPerformance(const std::string& sourceFile);
};

#endif // TEST_VALIDATOR_H
