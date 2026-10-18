// ---------------------------------------------------------------
//  HW2 参考答案 / 自检
//  - 用参考实现把两个三角形画出来,验证 z-buffer 的遮挡是否正确
//  - 打印关键采样点的颜色与深度,可以和自己的实现对照
//
//  编译: cmake -S . -B build -DHW2_SELFCHECK=ON && cmake --build build -j8
//  运行: ./build/selfcheck
// ---------------------------------------------------------------
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <iostream>

#include <Eigen/Dense>
#include <opencv2/opencv.hpp>

#include "Triangle.hpp"
#include "rasterizer.hpp"

// ---------------- 参考:点是否在三角形内 ----------------
static bool ref_inside(float x, float y, const Eigen::Vector3f* v)
{
    auto cross = [](float ax, float ay, float bx, float by) { return ax * by - ay * bx; };
    const float d0 = cross(v[1].x() - v[0].x(), v[1].y() - v[0].y(), x - v[0].x(), y - v[0].y());
    const float d1 = cross(v[2].x() - v[1].x(), v[2].y() - v[1].y(), x - v[1].x(), y - v[1].y());
    const float d2 = cross(v[0].x() - v[2].x(), v[0].y() - v[2].y(), x - v[2].x(), y - v[2].y());
    const bool any_neg = (d0 < 0) || (d1 < 0) || (d2 < 0);
    const bool any_pos = (d0 > 0) || (d1 > 0) || (d2 > 0);
    return !(any_neg && any_pos);     // 三个同号(或含 0)即在内部,与顶点顺序无关
}

// ---------------- 参考:光栅化 + z-buffer ----------------
static void ref_rasterize(rst::rasterizer& r, const Triangle& t)
{
    const Eigen::Vector3f& a = t.v[0];
    const Eigen::Vector3f& b = t.v[1];
    const Eigen::Vector3f& c = t.v[2];

    int minx = int(std::floor(std::min({ a.x(), b.x(), c.x() })));
    int maxx = int(std::ceil (std::max({ a.x(), b.x(), c.x() })));
    int miny = int(std::floor(std::min({ a.y(), b.y(), c.y() })));
    int maxy = int(std::ceil (std::max({ a.y(), b.y(), c.y() })));
    minx = std::max(minx, 0); maxx = std::min(maxx, r.buf_width()  - 1);
    miny = std::max(miny, 0); maxy = std::min(maxy, r.buf_height() - 1);

    const float area = (b.x() - a.x()) * (c.y() - a.y()) - (b.y() - a.y()) * (c.x() - a.x());
    if (std::abs(area) < 1e-6f) return;

    for (int y = miny; y <= maxy; ++y) {
        for (int x = minx; x <= maxx; ++x) {
            const float px = x + 0.5f, py = y + 0.5f;
            if (!ref_inside(px, py, t.v)) continue;

            // 重心坐标
            const float wa = ((b.x() - px) * (c.y() - py) - (b.y() - py) * (c.x() - px)) / area;
            const float wb = ((c.x() - px) * (a.y() - py) - (c.y() - py) * (a.x() - px)) / area;
            const float wc = 1.0f - wa - wb;

            const float z = wa * a.z() + wb * b.z() + wc * c.z();
            if (z >= r.get_depth(x, y)) continue;          // Z-Buffer:更近才写
            r.set_depth(x, y, z);
            r.set_pixel(Eigen::Vector3f(float(x), float(y), 0.f),
                        wa * t.color[0] + wb * t.color[1] + wc * t.color[2]);
        }
    }
}

static void ref_draw(rst::rasterizer& r, Triangle t,
                     const Eigen::Matrix4f& mvp, int sw, int sh)
{
    Eigen::Matrix4f viewport;
    viewport << sw / 2.0f, 0, 0, sw / 2.0f,
                0, -sh / 2.0f, 0, sh / 2.0f,
                0, 0, 1, 0,
                0, 0, 0, 1;
    for (int i = 0; i < 3; ++i) {
        Eigen::Vector4f clip = mvp * t.toVector4(i);
        Eigen::Vector4f ndc = clip / clip.w();
        Eigen::Vector4f s = viewport * Eigen::Vector4f(ndc.x(), ndc.y(), ndc.z(), 1.f);
        t.v[i] = Eigen::Vector3f(s.x(), s.y(), ndc.z());
    }
    ref_rasterize(r, t);
}

int main()
{
    const int W = 700, H = 700;
    Eigen::Vector3f eye_pos{ 0, 0, 5 };
    const float fov = 45.f, aspect = 1.f, zn = 0.1f, zf = 50.f;

    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
    view << 1,0,0,-eye_pos[0], 0,1,0,-eye_pos[1], 0,0,1,-eye_pos[2], 0,0,0,1;

    Eigen::Matrix4f P = Eigen::Matrix4f::Zero();
    {
        const float t = std::tan(fov / 2 / 180.f * float(M_PI)) * zn;
        P(0,0) = 2*zn/(2*t); P(1,1) = 2*zn/(2*t);
        P(2,2) = -(zf+zn)/(zf-zn); P(2,3) = -2*zf*zn/(zf-zn); P(3,2) = -1.f;
    }

    Triangle red, blue;
    red.setVertex(0, {  2,  0, -2 }); red.setVertex(1, {  0,  2, -2 }); red.setVertex(2, { -2,  0, -2 });
    red.setColors(255, 0, 0);
    blue.setVertex(0, { -3, -1, -8 }); blue.setVertex(1, {  3, -1, -8 }); blue.setVertex(2, {  0,  3, -8 });
    blue.setColors(0, 0, 255);

    rst::rasterizer r(W, H);
    r.clear(rst::Buffers(int(rst::Buffers::Color) | int(rst::Buffers::Depth)));
    const Eigen::Matrix4f mvp = P * view;
    ref_draw(r, red,  mvp, W, H);
    ref_draw(r, blue, mvp, W, H);

    auto buf = r.resolve();
    int lit = 0, redc = 0, bluec = 0;
    cv::Mat img(H, W, CV_8UC3, cv::Scalar(0,0,0));
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            const Eigen::Vector3f& c = buf[size_t(y)*W + x];
            if (c.x() > 1 || c.y() > 1 || c.z() > 1) ++lit;
            if (c.x() > 200 && c.z() < 60) ++redc;
            if (c.z() > 200 && c.x() < 60) ++bluec;
            img.at<cv::Vec3b>(y,x) = cv::Vec3b(cv::saturate_cast<uchar>(c.z()),
                                               cv::saturate_cast<uchar>(c.y()),
                                               cv::saturate_cast<uchar>(c.x()));
        }
    std::filesystem::create_directories("output");
    cv::imwrite("output/selfcheck.png", img);

    auto probe = [&](int x, int y, const char* name, const char* expect) {
        const Eigen::Vector3f c = r.get_pixel(x, y);
        std::printf("  %-18s (%3d,%3d) RGB=(%3.0f,%3.0f,%3.0f) depth=%.4f  期望 %s\n",
                    name, x, y, c.x(), c.y(), c.z(), r.get_depth(x, y), expect);
    };

    std::printf("着色像素   = %d\n", lit);
    std::printf("红色像素   = %d\n", redc);
    std::printf("蓝色像素   = %d\n\n", bluec);
    probe(350, 290, "红蓝重叠区", "红色(红更近)");
    probe(350, 382, "只有蓝区域", "蓝色");
    probe(350, 200, "只有红区域", "红色");
    probe(630, 350, "空白区域",   "黑色");

    std::printf("\nwrote output/selfcheck.png\n");
    return (lit > 1000 && redc > 100 && bluec > 100) ? 0 : 1;
}
