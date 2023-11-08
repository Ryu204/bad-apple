#include "port.hpp"
#include <CSerialPort/SerialPort.h>
#include <bits/chrono.h>
#include <chrono>
#include <functional>
#include <iostream>

namespace detail {
    Listener::Listener(itas109::CSerialPort& port) : m_port{port} {}
    auto Listener::onReadEvent(const char* port_name, unsigned int readBufferLen) -> void {
        if (readBufferLen == 0)
            return;
        std::vector<std::uint8_t> data(readBufferLen, 0);
        m_port.readData(data.data(), readBufferLen);
        if (m_callback)
            for (const auto& val : data)
                m_callback(val);
    }
}

Port::Port(std::size_t col, std::size_t row) 
    : m_column(col), m_row(row)
{
    m_listener.set_callback([this](std::uint8_t data) {
        std::lock_guard lock(m_queue_mutex);
        m_received.push(data);
    });
    m_port.connectReadEvent(&m_listener);
}

auto Port::open(const std::string& name, std::size_t baud) -> bool {
    m_port.init(name.c_str(), baud);
    m_port.open();
    return m_port.isOpen();
}

auto Port::close() -> void {
    if (m_port.isOpen())
        m_port.close();
}

auto Port::last_or_wait(float wait_millisecs) -> std::uint8_t {
    auto start_wait = std::chrono::steady_clock::now();
    auto now = start_wait;
    while (std::chrono::duration_cast<std::chrono::milliseconds>(now - start_wait).count() < wait_millisecs) {
        now = std::chrono::steady_clock::now();
        {
            std::lock_guard lock(m_queue_mutex);
            if (!m_received.empty()) {
                auto res = m_received.front();
                m_received.pop();
                return res;
            }
        }
    }
    return timeout_code;
}

auto Port::send(const std::uint8_t* data) -> bool {
    for (int i = 0; i < m_row; ++i) {
        auto signal = last_or_wait(1000.F);
        if (signal == timeout_code) {
            std::cerr << "MCU did not request line data in time" << std::endl;
            return false;
        }
        else if (signal != line_code) {
            std::cerr << std::format("MCU sent unknown signal ({}), stop sending", (int)signal) << std::endl;
            return false;
        }
        m_port.writeData(data + i * m_column, m_column);
    }
    return true;
}