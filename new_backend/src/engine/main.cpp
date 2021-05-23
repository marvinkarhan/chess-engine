#include "constants.h"
#include <iostream>
#include <chrono>

int main(int argc, char* argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    initConstants();
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "--- total runtime: " << elapsed.count() <<  " seconds ---" << std::endl;
    return 0;
}