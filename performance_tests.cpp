#include "performance_tests.h"
#include "ImageSearch.hpp"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <numeric>
#include <algorithm>
#include <Windows.h>

void RunTest(const char* testName, const char* imagePath, int left, int top, int right, int bottom, int tol = 0) {
    std::cout << "\nRunning test: " << testName << std::endl;
    std::cout << "----------------------------------------" << std::endl;

    int x = 0, y = 0;
    std::vector<double> times;
    const int NUM_ITERATIONS = 10;  // Increased iterations for better statistics

    try {
        // Warm-up run
        ImageSearch::Search(x, y, left, top, right, bottom, imagePath, tol);

        // Test runs
        for (int i = 1; i <= NUM_ITERATIONS; i++) {
            auto start = std::chrono::high_resolution_clock::now();
            int result = ImageSearch::Search(x, y, left, top, right, bottom, imagePath, tol);
            auto end = std::chrono::high_resolution_clock::now();
            
            double time = std::chrono::duration<double, std::milli>(end - start).count();
            times.push_back(time);
            
            if (i == 1) {  // Only print location for first successful find
                std::cout << "Result: " << (result == 1 ? 
                    "Found at (" + std::to_string(x) + ", " + std::to_string(y) + ")" : 
                    "Not found") << std::endl;
            }
        }

        if (!times.empty()) {
            double avgTime = std::accumulate(times.begin(), times.end(), 0.0) / times.size();
            double minTime = *std::min_element(times.begin(), times.end());
            double maxTime = *std::max_element(times.begin(), times.end());
            
            // Calculate standard deviation
            double sqSum = std::accumulate(times.begin(), times.end(), 0.0,
                [avgTime](double acc, double val) {
                    double diff = val - avgTime;
                    return acc + diff * diff;
                });
            double stdDev = std::sqrt(sqSum / times.size());

            std::cout << std::fixed << std::setprecision(2);
            std::cout << "\nPerformance Results:" << std::endl;
            std::cout << "  Average Time: " << avgTime << "ms" << std::endl;
            std::cout << "  Min Time:     " << minTime << "ms" << std::endl;
            std::cout << "  Max Time:     " << maxTime << "ms" << std::endl;
            std::cout << "  Std Dev:      " << stdDev << "ms" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Test error: " << e.what() << std::endl;
    }
}

int RunPerformanceTests() {
    try {
        std::cout << "ImageSearch Performance Tests" << std::endl;
        std::cout << "============================" << std::endl;

        // Define test parameters for quarter screen search
        int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);
        int quarterWidth = screenWidth / 2;
        int quarterHeight = screenHeight / 2;

        // Run tests
        RunTest("Small Image - Quarter Screen", "test_image_small.png", 
                0, 0, quarterWidth, quarterHeight);
                
        RunTest("Medium Image - Quarter Screen", "test_image_medium.png", 
                0, 0, quarterWidth, quarterHeight);
                
        RunTest("Small Image - With 5% Tolerance", "test_image_small.png", 
                0, 0, quarterWidth, quarterHeight, 13);

        std::cout << "\nAll tests completed successfully" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Performance test error: " << e.what() << std::endl;
        return 1;
    }
}
