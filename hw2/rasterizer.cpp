#include "rasterizer.hpp"
#include <algorithm>
#include <cmath>
#include <limits>

namespace rst {

static constexpr float kInf = std::numeric_limits<float>::infinity();

rasterizer::rasterizer(int w, int h) : width_(w), height_(h) {
    model_ = view_ = projection_ = Eigen::Matrix4f::Identity();
    resize_buffers();
}

void rasterizer::resize_buffers() {
    const size_t n = size_t(buf_width()) * size_t(buf_height());
    frame_buf_.assign(n, Eigen::Vector3f(0, 0, 0));
    depth_buf_.assign(n, kInf);
}

void rasterizer::set_sample_scale(int s) {
    sample_scale_ = std::max(1, s);
    resize_buffers();
}

void rasterizer::clear(Buffers buff) {
    const int b = int(buff);
    if (b & int(Buffers::Color)) std::fill(frame_buf_.begin(), frame_buf_.end(), Eigen::Vector3f(0, 0, 0));
    if (b & int(Buffers::Depth)) std::fill(depth_buf_.begin(), depth_buf_.end(), kInf);
}

void rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color) {
    const int x = int(point.x()), y = int(point.y());
    if (x < 0 || x >= buf_width() || y < 0 || y >= buf_height()) return;
    frame_buf_[size_t(y) * buf_width() + x] = color;
}

Eigen::Vector3f rasterizer::get_pixel(int x, int y) const {
    if (x < 0 || x >= buf_width() || y < 0 || y >= buf_height()) return Eigen::Vector3f(0, 0, 0);
    return frame_buf_[size_t(y) * buf_width() + x];
}

void rasterizer::set_depth(int x, int y, float z) {
    if (x < 0 || x >= buf_width() || y < 0 || y >= buf_height()) return;
    depth_buf_[size_t(y) * buf_width() + x] = z;
}

float rasterizer::get_depth(int x, int y) const {
    if (x < 0 || x >= buf_width() || y < 0 || y >= buf_height()) return kInf;
    return depth_buf_[size_t(y) * buf_width() + x];
}

void rasterizer::draw(Triangle& t) {
    const int sw = buf_width(), sh = buf_height();

    // NDC -> 屏幕像素;y 轴要翻(图像 y 朝下)
    Eigen::Matrix4f viewport;
    viewport << sw / 2.0f, 0.0f, 0.0f, sw / 2.0f,
                0.0f, -sh / 2.0f, 0.0f, sh / 2.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f;

    const Eigen::Matrix4f mvp = projection_ * view_ * model_;

    Triangle newtri = t;
    for (int i = 0; i < 3; ++i) {
        Eigen::Vector4f clip = mvp * t.toVector4(i);
        if (std::abs(clip.w()) < 1e-8f) return;          // 顶点落在相机平面上
        const Eigen::Vector4f ndc = clip / clip.w();      // 透视除法
        const Eigen::Vector4f s = viewport * Eigen::Vector4f(ndc.x(), ndc.y(), ndc.z(), 1.0f);
        newtri.v[i] = Eigen::Vector3f(s.x(), s.y(), ndc.z());
    }

    rasterize_triangle(newtri);
}

// ============================================================================
//  ↓↓↓                    作业 2 要你实现的部分                    ↓↓↓
// ============================================================================

bool rasterizer::insideTriangle(float x, float y, const Eigen::Vector3f* _v) const
{
    (void)x; (void)y; (void)_v;

    // TODO: 判断点 (x, y) 是否在三角形内
    // 思路 A:分别对三条边做叉积,三个结果同号即在内部(注意顶点顺序可能是顺时针也可能逆时针)
    // 思路 B:算重心坐标 wa/wb/wc,三个都 >= 0 即在内部

    return false;
}

void rasterizer::rasterize_triangle(const Triangle& t)
{
    (void)t;

    // TODO:
    //  1) 求三角形在屏幕上的包围盒,并 clamp 到 [0, buf_width()-1] x [0, buf_height()-1]
    //  2) 遍历包围盒里的每个像素中心 (x + 0.5, y + 0.5)
    //  3) 用 insideTriangle 判断在内;在的话算出以该点为中心的重心坐标 (wa, wb, wc)
    //  4) 深度 z = wa*z0 + wb*z1 + wc*z2,与 depth_buf_ 比较:更近才写入
    //     注意 NDC 里 z 越小越近(-1 是近平面,+1 是远平面)
    //  5) 颜色同样按重心坐标插值后写进 frame_buf_
}

// ============================================================================
//  ↑↑↑                    作业部分结束                            ↑↑↑
// ============================================================================

std::vector<Eigen::Vector3f> rasterizer::resolve() const {
    const int s = sample_scale_;
    std::vector<Eigen::Vector3f> out(size_t(width_) * size_t(height_));
    const int sw = buf_width();
    for (int y = 0; y < height_; ++y) {
        for (int x = 0; x < width_; ++x) {
            Eigen::Vector3f acc(0, 0, 0);
            for (int dy = 0; dy < s; ++dy)
                for (int dx = 0; dx < s; ++dx)
                    acc += frame_buf_[size_t(y * s + dy) * sw + (x * s + dx)];
            out[size_t(y) * width_ + x] = acc / float(s * s);
        }
    }
    return out;
}

} // namespace rst
