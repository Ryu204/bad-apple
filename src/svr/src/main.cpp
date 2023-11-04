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
    while (true) {
        for (int i = 0; i < 64; ++i) {
            port.send(data + i, 1);
            std::this_thread::sleep_for(std::chrono::microseconds(usecs));
        }
        std::cout << '.' << std::flush;
    }
}