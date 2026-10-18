#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <string>

#include <Eigen/Dense>
#include <opencv2/opencv.hpp>

#include "Triangle.hpp"
#include "rasterizer.hpp"

// ============================================================================
//  GAMES101 - Homework 2 : 光栅化与 Z-Buffer
//  框架已就绪,你要实现 rasterizer.cpp 里的两个函数(见文件中部标注)
// ============================================================================

// ---------- 下面这些是 HW1 的内容,这里直接给你,不用再写 ----------
Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    const float rad = rotation_angle / 180.0f * float(M_PI);
    const float c = std::cos(rad), s = std::sin(rad);
    model <<  c, -s, 0, 0,
              s,  c, 0, 0,
              0,  0, 1, 0,
              0,  0, 0, 1;
    return model;
}

Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0],
                 0, 1, 0, -eye_pos[1],
                 0, 0, 1, -eye_pos[2],
                 0, 0, 0, 1;
    return translate * view;
}

Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    const float n = zNear, f = zFar;
    const float t = std::tan(eye_fov / 2.0f / 180.0f * float(M_PI)) * n;
    const float b = -t, r = t * aspect_ratio, l = -r;

    Eigen::Matrix4f P = Eigen::Matrix4f::Zero();
    P(0, 0) = 2 * n / (r - l);      P(0, 2) = (r + l) / (r - l);
    P(1, 1) = 2 * n / (t - b);      P(1, 2) = (t + b) / (t - b);
    P(2, 2) = -(f + n) / (f - n);   P(2, 3) = -2 * f * n / (f - n);
    P(3, 2) = -1.0f;
    return P;
}

// ============================================================================

static cv::Mat frame_to_mat(const std::vector<Eigen::Vector3f>& buf, int w, int h)
{
    cv::Mat img(h, w, CV_8UC3);
    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x) {
            const Eigen::Vector3f& c = buf[size_t(y) * w + x];
            img.at<cv::Vec3b>(y, x) = cv::Vec3b(
                cv::saturate_cast<uchar>(c.z()),
                cv::saturate_cast<uchar>(c.y()),
                cv::saturate_cast<uchar>(c.x()));
        }
    return img;
}

int main(int argc, char** argv)
{
    const int W = 700, H = 700;
    Eigen::Vector3f eye_pos{ 0, 0, 5 };
    const float fov = 45.0f, aspect = 1.0f, zNear = 0.1f, zFar = 50.0f;

    // 两个三角形:红色近(z=-2),蓝色远(z=-8),在屏幕上部分重叠
    Triangle red;
    red.setVertex(0, {  2,  0, -2 });
    red.setVertex(1, {  0,  2, -2 });
    red.setVertex(2, { -2,  0, -2 });
    red.setColors(255, 0, 0);

    Triangle blue;
    blue.setVertex(0, { -3, -1, -8 });
    blue.setVertex(1, {  3, -1, -8 });
    blue.setVertex(2, {  0,  3, -8 });
    blue.setColors(0, 0, 255);

    // 命令行:无参数 -> 1x;数字 -> 该倍率超采样;both -> 1x 和 2x 各出一张
    int scales[2] = { 1, 1 };
    int n_scale = 1;
    bool both = false;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "both") both = true;
        else if (a == "help") {
            std::cout << "用法: ./build/hw2 [倍率|both]\n"
                         "  无参数  渲染 1x        -> output/hw2.png\n"
                         "  2       渲染 2x2 SSAA  -> output/hw2_2x.png\n"
                         "  both    两张都出\n";
            return 0;
        } else scales[0] = std::max(1, std::atoi(a.c_str()));
    }
    if (both) { scales[0] = 1; scales[1] = 2; n_scale = 2; }

    std::filesystem::create_directories("output");

    for (int k = 0; k < n_scale; ++k) {
        const int scale = scales[k];
        rst::rasterizer r(W, H);
        r.set_sample_scale(scale);
        r.clear(rst::Buffers(int(rst::Buffers::Color) | int(rst::Buffers::Depth)));

        r.set_model(get_model_matrix(0.0f));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(fov, aspect, zNear, zFar));

        r.draw(red);
        r.draw(blue);

        auto buf = r.resolve();

        // 统计:有多少像素被写入了颜色、深度缓冲的范围
        int lit = 0;
        float dmin = 1e9f, dmax = -1e9f;
        for (int y = 0; y < H; ++y)
            for (int x = 0; x < W; ++x) {
                const Eigen::Vector3f& c = buf[size_t(y) * W + x];
                if (c.x() > 1 || c.y() > 1 || c.z() > 1) ++lit;
                const float d = r.get_depth(x * scale, y * scale);
                if (d < 1e8f) { dmin = std::min(dmin, d); dmax = std::max(dmax, d); }
            }

        char name[64];
        if (scale == 1) std::snprintf(name, sizeof(name), "output/hw2.png");
        else            std::snprintf(name, sizeof(name), "output/hw2_%dx.png", scale);

        cv::imwrite(name, frame_to_mat(buf, W, H));
        std::printf("[%dx] 写入 %s   着色像素=%d   深度范围=[%.4f, %.4f]\n",
                    scale, name, lit, dmin, dmax);

        if (lit == 0) {
            std::cout << "[提示] 一个像素都没画出来 —— rasterize_triangle / insideTriangle\n"
                         "       还是空的(作业初始状态)。实现完就有画面了。\n";
        }
    }
    return 0;
}
