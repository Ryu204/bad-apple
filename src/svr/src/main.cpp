#include <bits/chrono.h>
#include <iostream>
#include <cstdint>
#include <format>
#include <ratio>
#include <string>
#include <thread>
#include <chrono>

#include "port.hpp"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Invalid arguments. Options:\nserver <device> <baudrate> <wait_time_us>" << std::endl;
        return -1;
    }
    auto* device = argv[1];
    std::size_t baud = std::stoi(std::string(argv[2]));
    std::size_t usecs = std::stoi(std::string(argv[3]));

    std::cout << "Arguments are valid, process to run..." << std::endl;

    using namespace std::chrono_literals;
    Port port;
    if (!port.open(device, baud)) {
        std::cerr << std::format("Cannot open device {}", device) << std::endl;
        return -1;
    }
    else {
        std::cout << std::format("Opened device {}", device) << std::endl;
    }

    std::uint8_t data[] = {
        0,0,0,0,0,0,0,0,
        0,0,1,1,1,1,0,0,
        0,1,1,1,1,1,1,1,
        1,1,8,1,1,1,8,1,
        1,1,1,1,1,1,1,1,
        1,1,8,8,8,8,8,1,
        0,1,1,8,8,8,1,1,
        0,0,1,1,1,1,1,0
    };
    std::size_t frame = 0;
    auto start = std::chrono::steady_clock::now();
    while (true) {
        frame++;
        if (port.wait() != 0xFF)
            break;
        std::cout << "Received request" << std::endl;
        port.send(data, 64, usecs);
        std::cout << "Sent frame " << frame << std::endl;
        if (frame == 100) {
            auto total_time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - start).count();
            std::cout << "FPS: " << 100.F / total_time << std::endl;
            break;
        }
    }
}