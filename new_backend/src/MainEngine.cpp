#include "engine/constants.h"
#include <iostream>
#include <chrono>

int main(int argc, char* argv[]) {
    auto start = std::chrono::high_resolution_clock::now();
    initConstants();
    for (int i = 0; i < 63; i++) {
        std::cout << SQUARE_BBS[i] << std::endl;
    }
    auto finish = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "--- total runtime: " << elapsed.count() <<  " seconds ---" << std::endl;
    return 0;
}