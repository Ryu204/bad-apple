#pragma once

#include <cstddef>

#include "ext/serial.hpp"

struct Port {
        auto open(const char* name, std::size_t baud) -> bool;
        auto close() -> void;
        auto send(const void* start, std::size_t num_bytes) -> void;
    private:
        serialib m_serial_port;
};