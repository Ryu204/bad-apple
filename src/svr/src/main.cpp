#include <iostream>
#include <fstream>
#include <cstdint>
#include <format>
#include <memory>
#include <string>
#include <thread>
#include <chrono>

#include "port.hpp"
#include "video.hpp"
#include "ext/include/glad/glad.h"
#include <SFML/Window.hpp>

using namespace std::chrono_literals;

Port port;
std::size_t baud, usecs;
char* device;
std::unique_ptr<Video> video;
std::size_t rendered_frames;
std::unique_ptr<sf::Window> window;
unsigned int VBO, VAO, EBO, original_texture;
unsigned int shader_program;
constexpr std::size_t PREVIEW_WIDTH = 640;
Video::Frame frame_data;

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
    if(!ok)
    {
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

    constexpr float vert[] = {
        // Position(x,y) - TexCoord(x, y)
        -0.5F,  0.5F,   0.F,    0.F,
        0.5F,   0.5F,   1.F,    0.F, 
        0.5F,   -0.5F,  1.F,    1.F,
        -0.5F,  -0.5F,  0.F,    1.F
    };
    const unsigned int indices[] = {
        0, 1, 2, 0, 2, 3
    };
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vert), vert, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, false, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, false, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glGenTextures(1, &original_texture);
    glBindTexture(GL_TEXTURE_2D, original_texture);
}

bool init(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Invalid arguments. Options:\nserver <device> <baudrate> <wait_time_us>" << std::endl;
        return false;
    }

    device = argv[1];
    baud = std::stoi(std::string(argv[2]));
    usecs = std::stoi(std::string(argv[3]));
    std::cout << "Arguments are valid, process to run..." << std::endl;
    if (!port.open(device, baud)) {
        std::cerr << std::format("Cannot open device {}", device) << std::endl;
        return false;
    }
    std::cout << std::format("Opened device {}", device) << std::endl;
    
    video = std::make_unique<Video>(VType::FILE, "rsc/bad-apple.mp4");
    video->set_size(PREVIEW_WIDTH, PREVIEW_WIDTH);

    init_opengl();

    return true;
}

bool fetch_image() {
    cv::waitKey(10); // Dummy to keep windows active
    frame_data = video->get();
    if (frame_data.empty())
        return false;
    cv::imshow(std::string("lol"), frame_data);
    return true;
}

void draw_result() {
    // cv::cvtColor(frame_data, frame_data, cv::COLOR_BGR2RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, frame_data.cols, frame_data.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, frame_data.ptr());
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindVertexArray(VAO);
    glUseProgram(shader_program);
    glUniform1i(glGetUniformLocation(shader_program, "image_texture"), 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
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
    glClearColor(1, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT);
    draw_result();
    window->display();
    return true;
}

int main(int argc, char* argv[]) {
    try{
        if (!init(argc, argv)) {
            return -1;
        }
        auto start = std::chrono::steady_clock::now();
        
        while (true) {
            bool ok = fetch_image() && display_result() && !sf::Keyboard::isKeyPressed(sf::Keyboard::Scan::Escape);
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
        return 0;
    }
    catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
}