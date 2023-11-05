#pragma once

#include <memory>
#include <cstdint>
#include <opencv4/opencv2/opencv.hpp>

enum class VType {
    CAMERA,
    FILE
};

struct Video {
        using Frame = cv::Mat;
        Video(VType type, const char* name);
        ~Video();
        Frame get();
        void set_size(std::size_t width, std::size_t height);
    private:
        cv::VideoCapture m_video;
};