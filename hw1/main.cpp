#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <string>

#include <Eigen/Dense>
#include <opencv2/opencv.hpp>

#include "Triangle.hpp"
#include "rasterizer.hpp"

// ============================================================================
//  GAMES101 - Homework 1 : 变换与投影
//  框架全部就绪,你只需要实现下面两个函数(main 之后的代码都不用改)
// ============================================================================

// ---------------------------------------------------------------------------
//  ① 模型变换:绕 Z 轴旋转 rotation_angle 度
//     Rz(θ) = [ cosθ  -sinθ  0  0 ]
//             [ sinθ   cosθ  0  0 ]
//             [   0     0    1  0 ]
//             [   0     0    0  1 ]
//     提醒:角度是【度】,sin/cos 要的是【弧度】,记得 angle / 180.0 * M_PI
// ---------------------------------------------------------------------------
Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();

    const float rad = rotation_angle / 180.0f * float(M_PI);   // 度 -> 弧度
    const float c = std::cos(rad), s = std::sin(rad);
    model <<  c, -s, 0, 0,
              s,  c, 0, 0,
              0,  0, 1, 0,
              0,  0, 0, 1;

    (void)rotation_angle;
    return model;
}

// ---------------------------------------------------------------------------
//  ② 投影变换:透视投影
//     步骤:视锥参数(l,r,b,t) -> 透视压缩 -> 正交投影到 NDC
//     关键:最后一行必须是 [0, 0, -1, 0],它让 w = -z,透视除法才成立
//     提醒:zNear / zFar 传进来是【正数】(相机到平面的距离),视图空间看向 -z
// ---------------------------------------------------------------------------
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
                                      float zNear, float zFar)
{
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // TODO: 在这里填上你的代码
    // 思路:先由 fov / aspect / zNear 求出近平面四边 l, r, b, t,
    //       再按下面 8 个非零元素逐个填进 4x4 矩阵(完整推导见 README 旁的 HW1-详解.md):
    //
    //   P(0,0) =  2n / (r - l)          P(0,2) = (r + l) / (r - l)
    //   P(1,1) =  2n / (t - b)          P(1,2) = (t + b) / (t - b)
    //   P(2,2) = -(f + n) / (f - n)     P(2,3) = -2fn / (f - n)
    //   P(3,2) = -1                     (这一项让 w = -z,透视除法才成立)
    //
    // 注意 n = zNear、f = zFar 是正数,矩阵里的 -(f+n)/(f-n) 已经处理了朝向 -z 的符号。

    const float fov = eye_fov / 180.0f * float(M_PI);   // 度 -> 弧度
    Eigen::Matrix4f persp = Eigen::Matrix4f::Zero();
    Eigen::Matrix4f T = Eigen::Matrix4f::Zero();
    Eigen::Matrix4f S = Eigen::Matrix4f::Zero();

    const float n = zNear, f = zFar;
    const float t = n * tan(fov / 2);
    const float b = -t;
    const float r = t * aspect_ratio;
    const float l = -r;

    persp << n, 0, 0, 0,
             0, n, 0, 0,
             0, 0, n + f, -n * f,
             0, 0, -1, 0;

    T << 1, 0, 0, -(r + l) / 2,
         0, 1, 0, -(t + b) / 2,
         0, 0, 1, 3 * (n + f) / 2,
         0, 0, 0, 1;

    S << 2 / (r - l), 0, 0, 0,
         0, 2 / (t - b), 0, 0,
         0, 0, 2 / (f - n), 0,
         0, 0, 0, 1;

    projection = S * T * persp;

    

    (void)eye_fov; (void)aspect_ratio; (void)zNear; (void)zFar;
    return projection;
}

// ---------------------------------------------------------------------------
//  ③ 视图变换(已给定,不用改):把相机平移到原点
// ---------------------------------------------------------------------------
Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0],
                 0, 1, 0, -eye_pos[1],
                 0, 0, 1, -eye_pos[2],
                 0, 0, 0, 1;

    view = translate * view;
    return view;
}

// ============================================================================
//  以下为框架代码
// ============================================================================

static cv::Mat frame_to_mat(const std::vector<Eigen::Vector3f>& buf, int w, int h)
{
    cv::Mat img(h, w, CV_8UC3);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            const Eigen::Vector3f& c = buf[size_t(y) * w + x];
            // frame buffer 存的是 RGB,OpenCV 是 BGR
            img.at<cv::Vec3b>(y, x) = cv::Vec3b(
                cv::saturate_cast<uchar>(c.z()),
                cv::saturate_cast<uchar>(c.y()),
                cv::saturate_cast<uchar>(c.x()));
        }
    }
    return img;
}

int main(int argc, char** argv)
{
    const int W = 700, H = 700;
    rst::rasterizer r(W, H);
    Eigen::Vector3f eye_pos{ 0, 0, 5 };

    // 课程给定的相机与视锥参数
    const float eye_fov = 45.0f, aspect_ratio = 1.0f;
    const float zNear = 0.1f, zFar = 50.0f;

    Triangle t;   // 红色三角形 (2,0,-2) (0,2,-2) (-2,0,-2)

    // ---- 命令行参数 ----
    //   无参数        : 从 0 到 350 度每 10 度一帧,存进 output/
    //   show          : 开窗口播动画(需要 WSLg 或真桌面)
    //   <数字>        : 只渲染这一个角度,存成 output/hw1_<角度>.png
    bool show = false;
    float single = -1.0f;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        if (a == "show") show = true;
        else single = std::stof(a);
    }

    // 打印一次三个矩阵,作业要求里有这一项
    {
        const Eigen::Matrix4f m = get_model_matrix(0.0f);
        const Eigen::Matrix4f v = get_view_matrix(eye_pos);
        const Eigen::Matrix4f p = get_projection_matrix(eye_fov, aspect_ratio, zNear, zFar);
        std::cout << "===== model (angle = 0) =====\n"     << m
                  << "\n===== view =====\n"               << v
                  << "\n===== projection =====\n"         << p
                  << "\n===== mvp =====\n"                << (p * v * m) << std::endl;

        if ((p - Eigen::Matrix4f::Identity()).cwiseAbs().maxCoeff() < 1e-6f &&
            (m - Eigen::Matrix4f::Identity()).cwiseAbs().maxCoeff() < 1e-6f) {
            std::cout << "[提示] model 与 projection 仍是单位阵(作业初始状态),\n"
                         "       三角形会被推到屏幕外,画面会是全黑。填完两个函数就正常了。\n"
                      << std::endl;
        }
    }

    std::filesystem::create_directories("output");

    auto render_one = [&](float angle) {
        r.clear();
        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(eye_fov, aspect_ratio, zNear, zFar));
        r.draw(t);
        return frame_to_mat(r.frame_buffer(), W, H);
    };

    if (single >= 0.0f) {
        cv::Mat img = render_one(single);
        char name[64];
        std::snprintf(name, sizeof(name), "output/hw1_%03d.png", int(single));
        cv::imwrite(name, img);
        std::cout << "wrote " << name << std::endl;
        return 0;
    }

    if (show) {
        for (float angle = 0; angle < 360; angle += 5) {
            cv::imshow("GAMES101 HW1", render_one(angle));
            if (cv::waitKey(30) >= 0) break;
        }
        cv::destroyAllWindows();
        return 0;
    }

    int written = 0;
    for (int angle = 0; angle < 360; angle += 10) {
        cv::Mat img = render_one(float(angle));
        char name[64];
        std::snprintf(name, sizeof(name), "output/hw1_%03d.png", angle);
        cv::imwrite(name, img);
        ++written;
    }
    std::cout << "wrote " << written << " frames to output/" << std::endl;
    return 0;
}
