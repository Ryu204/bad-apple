#include "port.hpp"

auto Port::open(const char* name, std::size_t baud) -> bool {
    m_serial_port.closeDevice();
    return m_serial_port.openDevice(name, baud);
}

auto Port::send(const void* start, std::size_t num_bytes) -> void {
    m_serial_port.writeBytes(start, num_bytes);
}