#include "ImageSearch.hpp"
#include "performance_tests.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        if (argc > 1 && std::string(argv[1]) == "--perf-test") {
            int testResult = RunPerformanceTests();
            std::cout << "Performance tests completed with status: " << testResult << std::endl;
            return testResult;
        }

        int x = 0, y = 0;
        int result = ImageSearch::Search(x, y, 0, 0, 1920, 1080, "test.png");
        
        if (result == 1) {
            std::cout << "Image found at: " << x << "," << y << std::endl;
        } else if (result == 0) {
            std::cout << "Image not found" << std::endl;
        } else {
            std::cout << "Error occurred" << std::endl;
            return 1;
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown fatal error" << std::endl;
        return 1;
    }
}