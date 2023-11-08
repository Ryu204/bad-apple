#include "video.hpp"
#include <memory>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <format>

Video::Video(VType type, const char* name) {
    if (type == VType::CAMERA) {
        if (m_video.open(0))
            std::cout << "Opened default camera" << std::endl;
        else
            std::cerr << "Cannot open default camera" << std::endl;
    }
    else {
        if (m_video.open(name))
            std::cout << "Opened video " << name << std::endl;
        else
            std::cerr << std::format("Cannot open video \"{}\"", name) << std::endl;
    }
}

Video::~Video() {
    m_video.release();
}

Video::Frame Video::get() {
    cv::Mat input;
    m_video.read(input);

    int size = std::min(input.cols, input.rows);
    int x = (input.cols - size) / 2;
    int y = (input.rows - size) / 2;
    cv::Rect roi(x, y, size, size);
    cv::Mat cropped = input(roi).clone();
    return cropped;
}

void Video::get_size(std::size_t* width, std::size_t* height) {
    *width = m_video.get(cv::CAP_PROP_FRAME_WIDTH);
    *height = m_video.get(cv::CAP_PROP_FRAME_HEIGHT);
}

auto Video::FPmS() -> float const {
    return m_video.get(cv::CAP_PROP_FPS) / 1000.F;
}