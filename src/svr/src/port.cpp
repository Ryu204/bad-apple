#include "port.hpp"
#include <iostream>

auto Port::open(const char* name, std::size_t baud) -> bool {
    std::cout << "Opening port " << name << std::endl;
    m_serial_port.init(name, baud);
    std::cout << "Initialized port " << name << std::endl;
    m_serial_port.open();
    std::cout << "Opened" << std::endl;
    return m_serial_port.isOpen();
}

auto Port::send(const void* start, std::size_t num_bytes) -> void {
    m_serial_port.writeData(start, num_bytes);
}