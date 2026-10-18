// HW0 冒烟测试:验证 Eigen 与 OpenCV 能联合编译、链接、出图
#include <Eigen/Dense>
#include <opencv2/opencv.hpp>
#include <cmath>
#include <iostream>

int main() {
    std::cout << "OpenCV version: " << CV_VERSION << std::endl;
    std::cout << "Eigen version:  " << EIGEN_WORLD_VERSION << "."
              << EIGEN_MAJOR_VERSION << "." << EIGEN_MINOR_VERSION << std::endl;

    // 1) Eigen:绕 Z 轴旋转 45 度,作用到 (1,0,0)
    const float angle = float(M_PI) / 9.0f;   // 20 度:让旋转在图上肉眼可见
    Eigen::Matrix3f R;
    R << std::cos(angle), -std::sin(angle), 0.0f,
         std::sin(angle),  std::cos(angle), 0.0f,
         0.0f,             0.0f,             1.0f;
    Eigen::Vector3f v(1.0f, 0.0f, 0.0f);
    Eigen::Vector3f rv = R * v;
    std::cout << "R * (1,0,0) = " << rv.transpose() << "  (expect 0.9397 0.3420 0)"
              << std::endl;

    // 2) OpenCV:把同一个旋转矩阵画出来
    const int W = 700, H = 700;
    cv::Mat img(H, W, CV_8UC3, cv::Scalar(16, 16, 16));
    const cv::Point2f c(W / 2.0f, H / 2.0f);
    const float side = 260.0f;

    std::vector<cv::Point> pts;
    for (int i = 0; i < 4; ++i) {
        const float a = float(i) * float(M_PI) / 2.0f;
        Eigen::Vector3f p(side * std::cos(a), side * std::sin(a), 1.0f);
        Eigen::Vector3f q = R * p;            // 复用上面算好的 Eigen 矩阵
        pts.push_back(cv::Point(int(c.x + q.x()), int(c.y - q.y())));
    }
    cv::polylines(img, pts, true, cv::Scalar(80, 200, 255), 3, cv::LINE_AA);
    cv::circle(img, c, 4, cv::Scalar(255, 255, 255), -1, cv::LINE_AA);
    cv::putText(img, "GAMES101 HW0 OK", cv::Point(210, 360),
                cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(255, 255, 255), 2, cv::LINE_AA);

    const std::string out = "hw0_smoke.png";
    if (!cv::imwrite(out, img)) {
        std::cerr << "imwrite failed" << std::endl;
        return 1;
    }
    std::cout << "wrote " << out << " (" << img.cols << "x" << img.rows << ")" << std::endl;
    return 0;
}
