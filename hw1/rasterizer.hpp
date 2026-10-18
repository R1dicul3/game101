#pragma once
#include <vector>
#include <Eigen/Dense>
#include "Triangle.hpp"

namespace rst {

// HW1 版光栅器:负责 MVP -> 透视除法 -> 视口变换 -> 填充三角形。
// 注意它【不做深度测试】(那是 HW2 的 Z-Buffer 内容),所以这里只有一个三角形也没关系。
class rasterizer {
public:
    rasterizer(int w, int h);

    void set_model(const Eigen::Matrix4f& m)      { model_ = m; }
    void set_view(const Eigen::Matrix4f& v)       { view_ = v; }
    void set_projection(const Eigen::Matrix4f& p) { projection_ = p; }

    void draw(Triangle& t);
    void clear();

    const std::vector<Eigen::Vector3f>& frame_buffer() const { return frame_buf_; }
    int width()  const { return width_; }
    int height() const { return height_; }

private:
    int width_, height_;
    std::vector<Eigen::Vector3f> frame_buf_;   // RGB, 0..255,下标 = y * width + x
    Eigen::Matrix4f model_, view_, projection_;
};

} // namespace rst
