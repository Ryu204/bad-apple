#pragma once

#include <cstdint>
#include <array>
#include <mutex>
#include <string>
#include <atomic>
#include <functional>
#include <queue>
#include "CSerialPort/SerialPort.h"

namespace detail {
    struct Listener : itas109::CSerialPortListener {
            Listener(itas109::CSerialPort& port);
            auto onReadEvent(const char* port_name, unsigned int readBufferLen) -> void override;
            template <typename Func>
            void set_callback(Func&& func) {
                m_callback = [func = std::forward<Func>(func)](std::uint8_t data) {
                    func(data);
                };
            }
        private:
            std::function<void(std::uint8_t)> m_callback;
            itas109::CSerialPort& m_port;
    };
}

class Port {
    public:
        static constexpr std::uint8_t timeout_code = 0xFD;
        static constexpr std::uint8_t line_code = 0xFE;
        static constexpr std::uint8_t frame_code = 0xFF;
        Port(std::size_t col, std::size_t row);
        auto open(const std::string& name, std::size_t baud) -> bool;
        auto close() -> void;
        auto last_or_wait(float wait_millisecs) -> std::uint8_t;
        auto send(const std::uint8_t* data) -> bool;
    private:
        using Data = std::uint8_t;
        itas109::CSerialPort m_port{};
        detail::Listener m_listener{m_port};
        std::queue<Data> m_received{};
        std::recursive_mutex m_queue_mutex{};

        std::size_t m_column;
        std::size_t m_row;
};