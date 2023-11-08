#pragma once

#include <cstddef>
#include <cstdint>
#include <functional>
#include <mutex>
#include <atomic>
#include <memory>

#include "CSerialPort/SerialPort.h"

namespace detail {
    struct ReadListener : itas109::CSerialPortListener {
            ReadListener(const char* port, itas109::CSerialPort* port_manager);
            ~ReadListener();
            template <typename T>
            void set_callback(T&& fn) {
                m_callback = [fn = std::forward<T>(fn), this](unsigned int num_bytes) {
                    fn(m_port_manager, num_bytes);
                };
            }
            void onReadEvent(const char *port, unsigned int num_bytes) override;
        private:
            const char* m_port_name;
            std::function<void(unsigned int num_bytes)> m_callback;
            itas109::CSerialPort* m_port_manager;
    };
}

struct Port {
        auto open(const char* name, std::size_t baud) -> bool;
        auto close() -> void;
        auto send(const void* start, std::size_t num_bytes, std::size_t delay_microseconds) -> bool;
        auto wait(float timeout = 0.F) -> std::uint8_t;
    private:
        itas109::CSerialPort m_serial_port;
        std::unique_ptr<detail::ReadListener> m_listener;
        std::atomic_bool m_is_waiting;
        std::atomic_char8_t m_value;
        std::atomic_bool m_received;
}; 