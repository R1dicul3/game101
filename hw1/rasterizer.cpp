#include "rasterizer.hpp"
#include <algorithm>
#include <cmath>

namespace rst {

rasterizer::rasterizer(int w, int h)
    : width_(w), height_(h), frame_buf_(size_t(w) * h) {
    model_ = view_ = projection_ = Eigen::Matrix4f::Identity();
}

void rasterizer::clear() {
    for (auto& c : frame_buf_) c = Eigen::Vector3f(0, 0, 0);
}

// 二维叉积 = 两条边张成的平行四边形面积(有符号)
static inline float cross2(float ax, float ay, float bx, float by) {
    return ax * by - ay * bx;
}

void rasterizer::draw(Triangle& t) {
    // ---- 1. 把 NDC 映射到屏幕像素 ----
    //  x: [-1,1] -> [0, W]      y: [-1,1] -> [H, 0]  (图像 y 轴朝下,所以 y 要翻)
    Eigen::Matrix4f viewport;
    viewport << width_ / 2.0f, 0.0f, 0.0f, width_ / 2.0f,
                0.0f, -height_ / 2.0f, 0.0f, height_ / 2.0f,
                0.0f, 0.0f, 1.0f, 0.0f,
                0.0f, 0.0f, 0.0f, 1.0f;

    const Eigen::Matrix4f mvp = projection_ * view_ * model_;

    Triangle newtri = t;
    for (int i = 0; i < 3; ++i) {
        Eigen::Vector4f clip = mvp * t.toVector4(i);

        // ---- 2. 透视除法(这一行就是"近大远小"真正发生的地方)----
        if (std::abs(clip.w()) < 1e-8f) {
            // 顶点落在相机平面上,整帧交给框架前先判掉
            return;
        }
        const Eigen::Vector4f ndc = clip / clip.w();

        // ---- 3. 视口变换 ----
        const Eigen::Vector4f screen = viewport * Eigen::Vector4f(ndc.x(), ndc.y(), ndc.z(), 1.0f);
        newtri.v[i] = Eigen::Vector3f(screen.x(), screen.y(), ndc.z());
    }

    // ---- 4. 填充三角形(包围盒 + 重心坐标)----
    const Eigen::Vector3f& a = newtri.v[0];
    const Eigen::Vector3f& b = newtri.v[1];
    const Eigen::Vector3f& c = newtri.v[2];

    int minx = int(std::floor(std::min({ a.x(), b.x(), c.x() })));
    int maxx = int(std::ceil (std::max({ a.x(), b.x(), c.x() })));
    int miny = int(std::floor(std::min({ a.y(), b.y(), c.y() })));
    int maxy = int(std::ceil (std::max({ a.y(), b.y(), c.y() })));

    minx = std::max(minx, 0);  maxx = std::min(maxx, width_ - 1);
    miny = std::max(miny, 0);  maxy = std::min(maxy, height_ - 1);

    const float area = cross2(b.x() - a.x(), b.y() - a.y(),
                              c.x() - a.x(), c.y() - a.y());
    if (std::abs(area) < 1e-6f) return;   // 退化三角形

    for (int y = miny; y <= maxy; ++y) {
        for (int x = minx; x <= maxx; ++x) {
            const float px = x + 0.5f, py = y + 0.5f;   // 取像素中心
            const float wa = cross2(b.x() - px, b.y() - py, c.x() - px, c.y() - py) / area;
            const float wb = cross2(c.x() - px, c.y() - py, a.x() - px, a.y() - py) / area;
            const float wc = 1.0f - wa - wb;
            if (wa < 0 || wb < 0 || wc < 0) continue;   // 在三角形外

            // 顶点色插值(HW1 里就是一个纯色三角形,但这样写后面着色器直接复用)
            const Eigen::Vector3f col = wa * t.color[0] + wb * t.color[1] + wc * t.color[2];
            frame_buf_[size_t(y) * width_ + x] = col;
        }
    }
}

} // namespace rst
