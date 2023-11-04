#pragma once

#include <cstddef>

#include "CSerialPort/SerialPort.h"

struct Port {
        auto open(const char* name, std::size_t baud) -> bool;
        auto close() -> void;
        auto send(const void* start, std::size_t num_bytes) -> void;
    private:
        itas109::CSerialPort m_serial_port{};
}; 