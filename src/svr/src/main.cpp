/////////////////////////////////////////////////////
//
// bad-apple on STC89C52 microcontroller
// Copyright (C) 2023 Nguyen Anh Bao (nguyenanhbao2356@gmail.com)
// The source code is delivered under MIT license.
//
/////////////////////////////////////////////////////

#include <cstring>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <format>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <chrono>

#include "port.hpp"
#include "video.hpp"
#include "ext/include/glad/glad.h"
#include <SFML/Window.hpp>
#include <SFML/System.hpp>

using namespace std::chrono_literals;

Port port{8, 8};
std::size_t baud;
float threshold[4];
char* device;
std::string resource;
std::unique_ptr<Video> video;
std::size_t rendered_frames;
std::unique_ptr<sf::Window> window;
unsigned int VBO, VAO, EBO, original_texture, SSBO;
std::array<std::uint8_t, 64> data, data_buffer;
std::array<int, 64> data_big;
unsigned int shader_program;
constexpr std::size_t PREVIEW_WIDTH = 640;
Video::Frame frame_data;
std::size_t time_per_frame = 1;

std::recursive_mutex matrix_mutex, window_mutex;

unsigned int create_shader(GLenum shader_type, const char* filename) {
    unsigned int result = glCreateShader(shader_type);
    std::stringstream source;
    source << std::ifstream(filename).rdbuf();
    auto source_code = source.str();
    auto* ptr = source_code.c_str();
    glShaderSource(result, 1, &ptr, nullptr);
    glCompileShader(result);
    int ok;
    char infoLog[512];
    glGetShaderiv(result, GL_COMPILE_STATUS, &ok);
    if(!ok) {
        glGetShaderInfoLog(result, 512, NULL, infoLog);
        std::cerr << std::format("Cannot compile shader in \"{}\":\n{}", filename, infoLog) << std::endl;
    }
    return result;
}

void init_opengl() {
    sf::ContextSettings settings {0, 0, 0, 4, 6};
    window = std::make_unique<sf::Window>(sf::VideoMode(PREVIEW_WIDTH, PREVIEW_WIDTH), "Preview", sf::Style::Default, settings);
    if (!gladLoadGL()) {
        std::cerr << "Cannot load GL" << std::endl;
    }
    glViewport(0, 0, PREVIEW_WIDTH, PREVIEW_WIDTH);

    auto vshader = create_shader(GL_VERTEX_SHADER, "rsc/vertice.vert");
    auto fshader = create_shader(GL_FRAGMENT_SHADER, "rsc/fragment.frag");
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vshader);
    glAttachShader(shader_program, fshader);
    glLinkProgram(shader_program);
    int okay;
    char infoLog[512];
    glGetProgramiv(shader_program, GL_LINK_STATUS, &okay);
    if(!okay) {
        glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
        std::cerr << "Cannot link shader. Reasons:\n" << infoLog << std::endl;
    }
    glDeleteShader(vshader);
    glDeleteShader(fshader);
    glUseProgram(shader_program);
    glUniform1fv(glGetUniformLocation(shader_program, "threshold"), 4, threshold);

    constexpr float vert[] = {
        // Position(x,y) - TexCoord(x, y)
        -1.0F,  1.F,   0.F,    0.F,
        1.F,   1.F,   1.F,    0.F, 
        1.F,   -1.F,  1.F,    1.F,
        -1.F,  -1.F,  0.F,    1.F
    };
    const unsigned int indices[] = {
        0, 1, 2, 0, 2, 3
    };
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenBuffers(1, &SSBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 64 * sizeof(int), nullptr, GL_DYNAMIC_COPY);

    glVertexAttribPointer(0, 2, GL_FLOAT, false, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glGenTextures(1, &original_texture);
    glBindTexture(GL_TEXTURE_2D, original_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

bool init(int argc, char* argv[]) {
    if (argc != 8) {
        std::cerr << "Invalid arguments. Argument list (8):\nserver <device> <baudrate> <brightness_threshold_1/2/3/4> <-f<filename>/-c>" << std::endl;
        return false;
    }

    device = argv[1];
    baud = std::stoi(std::string(argv[2]));
    threshold[0] = std::stof(std::string(argv[3]));
    threshold[1] = std::stof(std::string(argv[4]));
    threshold[2] = std::stof(std::string(argv[5]));
    threshold[3] = std::stof(std::string(argv[6]));
    resource = argv[7];
    std::cout << "Arguments are valid, process to run..." << std::endl;
    if (!port.open(device, baud)) {
        std::cerr << std::format("Cannot open device {}", device) << std::endl;
        return false;
    }
    std::cout << std::format("Opened device {}", device) << std::endl;
    
    if (resource.starts_with("-c")) {
        video = std::make_unique<Video>(VType::CAMERA, nullptr);
    }
    else if (resource.starts_with("-f")) {
        resource.erase(0, 2);
        video = std::make_unique<Video>(VType::FILE, resource.c_str());
        time_per_frame = 1.F / video->FPmS();
    }
    else {
        std::cout << "Filename or camera use was not specified, use -f or -c" << std::endl;
        return false;
    }

    init_opengl();

    return true;
}

bool fetch_image() {
    frame_data = video->get();
    if (frame_data.empty())
        return false;
    cv::imshow(std::string("Video capture"), frame_data);
    cv::waitKey(time_per_frame); // Dummy to keep windows active
    return true;
}

void draw_result() {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_data.cols, frame_data.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, frame_data.ptr());
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindVertexArray(VAO);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, SSBO);
    glUseProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, "image_texture"), 0);
    float image_size[] = {(float)frame_data.cols, (float)frame_data.rows};
    glUniform2fv(glGetUniformLocation(shader_program, "image_size"), 1, image_size);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void process_image() {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SSBO);
    std::fill(data_big.begin(), data_big.end(), 0);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 64 * sizeof(int), data_big.data());
    draw_result();
    glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 64 * sizeof(int), data_big.data());
    {
        std::lock_guard lock(matrix_mutex);
        for (int i = 0; i < 64; ++i)
            data_buffer[i] = static_cast<std::uint8_t>(data_big[i]);
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
        std::cout << "Data will be sent:\n";
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j)
                std::cout << data_big[i* 8 + j] << '\t';
            std::cout << std::endl;
        }
    }
}

bool display_result() {
    if (!window->isOpen())
        return false;
    sf::Event event;
    while (window->pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                window->close();
                break;
            default:
                break;
        }
    }
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    process_image();
    window->display();
    return true;
}

int main(int argc, char* argv[]) {
    try{
        if (!init(argc, argv)) {
            return -1;
        }
        auto start = std::chrono::steady_clock::now();
        
        auto port_handle = [&]() {
            std::cout << "Starting communication, waiting for request signal..." << std::endl;
            while (true) {
                rendered_frames++;
                auto signal = port.last_or_wait(5000.F);
                if (signal == Port::timeout_code) {
                    std::cerr << "No signal from MCU, stop sending" << std::endl;
                    break;
                }
                if (signal != Port::frame_code) {
                    std::cerr << std::format("Expected frame signal but got {}, stop sending", (int)signal) << std::endl;
                    break;
                }
                {
                    std::lock_guard lock(window_mutex);
                    if (window.get() == nullptr || !window->isOpen()) {
                        std::cout << "Window was closed, stop sending" << std::endl;
                        break;
                    }
                }
                std::cout << "Received request" << std::endl;
                {
                    std::lock_guard lock(matrix_mutex);
                    data = data_buffer;
                }
                auto ok = port.send(data.data());
                if (!ok) {
                    std::cerr << std::format("Cannot send frame {}", rendered_frames) << std::endl;
                    break;
                }
                std::cout << std::format("Sent frame {}", rendered_frames) << std::endl;
            }
        };

        std::thread port_communication{port_handle};

        while (true) {
            bool ok = sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Escape) == false && fetch_image();
            {
                std::lock_guard lock{window_mutex};
                ok = ok && display_result();
            }
            if (!ok)
                break;
        }

        // std::uint8_t data[] = {
        //     0,0,0,0,0,0,0,0,
        //     0,0,1,1,1,1,0,0,
        //     0,1,1,1,1,1,1,1,
        //     1,1,8,1,1,1,8,1,
        //     1,1,1,1,1,1,1,1,
        //     1,1,8,8,8,8,8,1,
        //     0,1,1,8,8,8,1,1,
        //     0,0,1,1,1,1,1,0
        // };

        // while (true) {
        //     rendered_frames++;
        //     if (port.wait() != 0xFF)
        //         break;
        //     std::cout << "Received request" << std::endl;
        //     port.send(data, 64, usecs);
        //     std::cout << "Sent frame " << rendered_frames << std::endl;
        // }

        port.close();
        video.reset();
        window.reset();
        cv::destroyAllWindows();
        port_communication.join();
        return 0;
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}