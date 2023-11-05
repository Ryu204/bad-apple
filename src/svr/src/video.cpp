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
            std::cout << std::format("Cannot open video \"{}\"", name) << std::endl;
    }
}

Video::~Video() {
    m_video.release();
}

Video::Frame Video::get() {
    cv::Mat frame;
    m_video.read(frame);
    return frame;
}

void Video::set_size(std::size_t width, std::size_t height) {
    m_video.set(cv::CAP_PROP_FRAME_WIDTH, width);
    m_video.set(cv::CAP_PROP_FRAME_HEIGHT, height);
}

