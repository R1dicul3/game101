#pragma once
#include <Eigen/Dense>

// 三角形:顶点 + 顶点颜色(顶点色会在光栅化时按重心坐标插值)
class Triangle {
public:
    Eigen::Vector3f v[3];
    Eigen::Vector3f color[3];

    Triangle() {
        v[0] = { 0, 0, 0 };
        v[1] = { 0, 0, 0 };
        v[2] = { 0, 0, 0 };
        color[0] = color[1] = color[2] = Eigen::Vector3f(255, 255, 255);
    }

    Eigen::Vector4f toVector4(int i) const {
        return Eigen::Vector4f(v[i].x(), v[i].y(), v[i].z(), 1.0f);
    }

    void setVertex(int ind, const Eigen::Vector3f& ver) { v[ind] = ver; }
    void setColor(int ind, float r, float g, float b) { color[ind] = { r, g, b }; }
    void setColors(float r, float g, float b) {
        for (int i = 0; i < 3; ++i) color[i] = { r, g, b };
    }
};
