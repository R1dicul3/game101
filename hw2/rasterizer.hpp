#pragma once
#include <vector>
#include <Eigen/Dense>
#include "Triangle.hpp"

namespace rst {

enum class Buffers { Color = 1, Depth = 2 };

class rasterizer {
public:
    rasterizer(int w, int h);

    void set_model(const Eigen::Matrix4f& m)      { model_ = m; }
    void set_view(const Eigen::Matrix4f& v)       { view_ = v; }
    void set_projection(const Eigen::Matrix4f& p) { projection_ = p; }

    void draw(Triangle& t);
    void clear(Buffers buff);

    // ---------- HW2 要你实现的两个函数 ----------
    // 判断点 (x, y) 是否在三角形 _v[0..2] 内
    // 提示:三次叉积同号 / 或用重心坐标判断三个分量都 >= 0。x、y 是【像素中心】的浮点坐标。
    bool insideTriangle(float x, float y, const Eigen::Vector3f* _v) const;

    // 把【已经过视口变换】的三角形填进 frame_buf_,同时做深度测试
    // t.v[i] = (屏幕 x, 屏幕 y, NDC 深度 z),注意 z 已归一化到 [-1, 1]
    void rasterize_triangle(const Triangle& t);

    // ---------- 工具函数(已实现) ----------
    void set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color);
    Eigen::Vector3f get_pixel(int x, int y) const;
    float get_depth(int x, int y) const;
    void  set_depth(int x, int y, float z);

    // 超采样倍数:1 = 不抗锯齿,2 = 2x2 SSAA(提高部分)
    void set_sample_scale(int s);
    int  sample_scale() const { return sample_scale_; }

    // 把 sample_scale 倍的缓冲降采样成 width x height 的结果
    std::vector<Eigen::Vector3f> resolve() const;

    int width()  const { return width_; }
    int height() const { return height_; }
    int buf_width()  const { return width_  * sample_scale_; }
    int buf_height() const { return height_ * sample_scale_; }

private:
    void resize_buffers();

    int width_, height_;
    int sample_scale_ = 1;
    std::vector<Eigen::Vector3f> frame_buf_;   // RGB 0..255
    std::vector<float> depth_buf_;             // NDC 深度,初始 +inf
    Eigen::Matrix4f model_, view_, projection_;
};

} // namespace rst
