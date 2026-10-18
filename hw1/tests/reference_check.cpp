// ---------------------------------------------------------------
//  HW1 参考实现 / 自检
//  1) 用参考矩阵验证 rasterizer 整条管线走得通
//  2) 打印关键数值,可以和手算结果对照
//  编译: cmake -S . -B build -DHW1_SELFCHECK=ON && cmake --build build -j8
//  运行: ./build/selfcheck
// ---------------------------------------------------------------
#include <cmath>
#include <cstdio>
#include <iostream>
#include <filesystem>

#include <Eigen/Dense>
#include <opencv2/opencv.hpp>

#include "Triangle.hpp"
#include "rasterizer.hpp"

Eigen::Matrix4f ref_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    const float rad = rotation_angle / 180.0f * float(M_PI);
    const float c = std::cos(rad), s = std::sin(rad);
    model << c, -s, 0, 0,
             s,  c, 0, 0,
             0,  0, 1, 0,
             0,  0, 0, 1;
    return model;
}

Eigen::Matrix4f ref_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    const float n = zNear, f = zFar;
    const float half_fov = eye_fov / 2.0f / 180.0f * float(M_PI);
    const float t = std::tan(half_fov) * n;   // 近平面半高
    const float b = -t;
    const float r = t * aspect_ratio;
    const float l = -r;

    Eigen::Matrix4f P = Eigen::Matrix4f::Zero();
    P(0, 0) = 2 * n / (r - l);
    P(0, 2) = (r + l) / (r - l);
    P(1, 1) = 2 * n / (t - b);
    P(1, 2) = (t + b) / (t - b);
    P(2, 2) = -(f + n) / (f - n);
    P(2, 3) = -2 * f * n / (f - n);
    P(3, 2) = -1.0f;                          // w = -z,透视除法全靠它
    return P;
}

int main()
{
    const float fov = 45.0f, aspect = 1.0f, zn = 0.1f, zf = 50.0f;
    Eigen::Vector3f eye_pos{ 0, 0, 5 };

    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
    view << 1, 0, 0, -eye_pos[0],
            0, 1, 0, -eye_pos[1],
            0, 0, 1, -eye_pos[2],
            0, 0, 0, 1;

    const Eigen::Matrix4f P = ref_projection_matrix(fov, aspect, zn, zf);
    std::cout << "projection =\n" << P << std::endl;

    // 顶点 (2,0,-2) 经 view 变成 (2,0,-7),期望 NDC ≈ (0.6898, 0, 0.9754)
    Triangle t;
    const Eigen::Vector4f clip = P * view * t.toVector4(0);
    const Eigen::Vector4f ndc = clip / clip.w();
    std::printf("vertex0 NDC = (%.4f, %.4f, %.4f)\n", ndc.x(), ndc.y(), ndc.z());
    std::printf("expect       = (0.6898, 0.0000, 0.9754)\n\n");

    const int W = 700, H = 700;
    rst::rasterizer r(W, H);
    r.clear();
    r.set_model(ref_model_matrix(0.0f));
    r.set_view(view);
    r.set_projection(P);
    r.draw(t);

    std::filesystem::create_directories("output");
    const auto& buf = r.frame_buffer();
    int lit = 0;
    cv::Mat img(H, W, CV_8UC3, cv::Scalar(0, 0, 0));
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            const Eigen::Vector3f& c = buf[size_t(y) * W + x];
            if (c.x() > 1 || c.y() > 1 || c.z() > 1) ++lit;
            img.at<cv::Vec3b>(y, x) = cv::Vec3b(
                cv::saturate_cast<uchar>(c.z()),
                cv::saturate_cast<uchar>(c.y()),
                cv::saturate_cast<uchar>(c.x()));
        }
    cv::imwrite("output/selfcheck.png", img);
    std::printf("lit pixels = %d (期望约 10 万量级,不是 0 也不是满屏)\n", lit);
    std::printf("wrote output/selfcheck.png\n");
    return lit > 1000 ? 0 : 1;
}
