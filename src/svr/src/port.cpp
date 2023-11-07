#include "port.hpp"
#include <CSerialPort/SerialPort.h>
#include <CSerialPort/SerialPortListener.h>
#include <cstring>
#include <iostream>
#include <functional>
#include <mutex>
#include <ratio>
#include <string>
#include <cassert>
#include <thread>
#include <chrono>

namespace detail {
    ReadListener::ReadListener(const char* port, itas109::CSerialPort* port_manager) 
        : m_port_name{port}
        , m_port_manager{port_manager}
    {
        m_port_manager->connectReadEvent(this);
    }

    ReadListener::~ReadListener() {
        m_port_manager->disconnectReadEvent();
    }
            
    void ReadListener::onReadEvent(const char *port, unsigned int num_bytes) {
        if (std::strcmp(m_port_name, port) != 0 || num_bytes == 0)
            return;
        m_callback(num_bytes);
    }
}

auto Port::open(const char* name, std::size_t baud) -> bool {
    std::cout << "Opening port " << name << std::endl;
    m_serial_port.init(name, baud);
    if (!m_serial_port.open()) 
        return false;

    m_listener = std::make_unique<detail::ReadListener>(name, &m_serial_port);
    m_listener->set_callback([this](itas109::CSerialPort* port, unsigned int num_butes){
        std::uint8_t res;
        port->readData(&res, 1);
        m_value.store(res);
        m_received.store(true);
    });
    m_is_waiting.store(false);
    m_received.store(false);
    
    std::cout << "Initialized port " << name << std::endl;
    return m_serial_port.isOpen();
}

auto Port::close() -> void {
    m_serial_port.close();
}

auto Port::send(const void* start, std::size_t num_bytes, std::size_t delay_microseconds) -> void {
    m_is_waiting.wait(true);
    for (int i = 0; i < num_bytes; ++i) {
        m_serial_port.writeData((const char*)start + i, 1);
        // std::this_thread::sleep_for(std::chrono::microseconds(delay_microseconds));
        auto time = std::chrono::steady_clock::now();
        while (true) {
        double past = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - time).count();
        if (past >= delay_microseconds)
            break;
        }
    }
}

auto Port::wait(float timeout) -> std::uint8_t {
    m_is_waiting.store(true);
    auto time = std::chrono::steady_clock::now();
    while (!m_received.load()) {
        double past = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - time).count();
        if (past >= timeout)
            return 0xFE;
    }
    m_is_waiting.store(false);
    auto res = m_value.load();
    m_received.store(false);
    return res;
}