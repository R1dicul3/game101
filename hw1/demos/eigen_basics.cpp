// Eigen 语法小样:逗号初始化到底是按什么顺序填的?
// 编译: g++ -std=c++17 -I/usr/include/eigen3 eigen_basics.cpp -o /tmp/demo && /tmp/demo
#include <Eigen/Dense>
#include <cmath>
#include <iostream>

int main()
{
    std::cout << "---------- 1) Identity():单位阵 ----------\n";
    Eigen::Matrix4f I = Eigen::Matrix4f::Identity();
    std::cout << I << "\n";

    std::cout << "\n---------- 2) 逗号初始化按【行】填 ----------\n";
    Eigen::Matrix2f A;
    A << 1, 2,      // 写完 2 个就换行了,但换行没有任何语义,纯粹好看
         3, 4;
    std::cout << "A << 1,2,3,4  得到:\n" << A << "\n";
    std::cout << "(如果按列填,这里会是 [1 3; 2 4],但实际是 [1 2; 3 4])\n";

    std::cout << "\n---------- 3) 4x4 旋转矩阵 ----------\n";
    const float angle = 30.0f;
    const float rad = angle / 180.0f * float(M_PI);
    const float c = std::cos(rad), s = std::sin(rad);
    Eigen::Matrix4f R = Eigen::Matrix4f::Identity();
    R <<  c, -s, 0, 0,
          s,  c, 0, 0,
          0,  0, 1, 0,
          0,  0, 0, 1;
    std::cout << R << "\n";
    std::cout << "cos30 = " << c << ",  sin30 = " << s << "\n";

    std::cout << "\n---------- 4) 只填某个子块 .block<>() ----------\n";
    Eigen::Matrix4f B = Eigen::Matrix4f::Identity();
    B.block<3, 3>(0, 0) << 1, 2, 3,
                           4, 5, 6,
                           7, 8, 9;
    std::cout << B << "\n";

    std::cout << "\n---------- 5) 单个元素读写用 () ----------\n";
    R(0, 0) = 99.0f;                       // 行, 列  都是 0 起
    std::cout << "R(0,0) = " << R(0, 0) << "\n";

    std::cout << "\n---------- 6) 矩阵乘向量 ----------\n";
    Eigen::Matrix4f R2 = Eigen::Matrix4f::Identity();
    const float a2 = 90.0f / 180.0f * float(M_PI);
    R2 << std::cos(a2), -std::sin(a2), 0, 0,
          std::sin(a2),  std::cos(a2), 0, 0,
          0, 0, 1, 0,
          0, 0, 0, 1;
    Eigen::Vector4f v(1, 0, 0, 1);
    std::cout << "R(90) * (1,0,0,1) = " << (R2 * v).transpose() << "   (期望 0 1 0 1)\n";
    return 0;
}
