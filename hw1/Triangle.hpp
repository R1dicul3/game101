#pragma once
#include <Eigen/Dense>
#include <array>

// HW1 只需要顶点 + 颜色;法线/纹理是 HW3 才用到的,这里不引入。
class Triangle {
public:
    Eigen::Vector3f v[3];
    Eigen::Vector3f color[3];

    Triangle() {
        // 课程给的初始三角形:z = -2,位于视线负方向,顶点顺序保证是正面
        v[0] = { 2, 0, -2 };
        v[1] = { 0, 2, -2 };
        v[2] = { -2, 0, -2 };
        color[0] = { 255, 0, 0 };
        color[1] = { 0, 255, 0 };
        color[2] = { 0, 0, 255 };
    }

    // 把第 i 个顶点变成齐次坐标,方便左乘 4x4 矩阵
    Eigen::Vector4f toVector4(int i) const {
        return Eigen::Vector4f(v[i].x(), v[i].y(), v[i].z(), 1.0f);
    }

    void setVertex(int ind, const Eigen::Vector3f& ver) { v[ind] = ver; }
    void setColor(int ind, float r, float g, float b) { color[ind] = { r, g, b }; }
};
