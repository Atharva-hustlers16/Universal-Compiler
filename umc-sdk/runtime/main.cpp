#include "Runtime.h"
#include <iostream>
#include <vector>
#include <string>

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: urun <executable> [args...]" << std::endl;
        return 1;
    }

    Runtime runtime;

    std::string executablePath = argv[1];
    if (!runtime.loadExecutable(executablePath)) {
        return 1;
    }

    // Execute with provided arguments
    std::vector<char*> execArgs;
    for (int i = 1; i < argc; ++i) {
        execArgs.push_back(argv[i]);
    }
    execArgs.push_back(nullptr);  // Null terminate

    if (!runtime.execute(argc - 1, execArgs.data())) {
        return 1;
    }

    return 0;
}
