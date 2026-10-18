# HW1 — 变换与投影

框架已经搭好,你只需要实现 @main.cpp@ 里的两个函数:

| 函数 | 作用 | 提示 |
|---|---|---|
| @get_model_matrix(rotation_angle)@ | 绕 Z 轴旋转,角度单位是**度** | 先转弧度;4x4 齐次矩阵右上角保持 0 |
| @get_projection_matrix(fov, aspect, zNear, zFar)@ | 透视投影 | 三段式:视锥参数 → 透视压缩 → 正交投影 |

@get_view_matrix@ 和光栅器(@rasterizer.cpp@)都已经写好,**不需要改动**。

## 跑起来

@@@bash
bash run.sh          # 编译 + 渲染 36 帧到 output/
bash run.sh 45       # 只渲染 45 度那一帧
bash run.sh show     # 开窗口播动画(WSLg 可用,Ctrl+C 退出)
@@@

输出在 @output/hw1_XXX.png@,用 @explorer.exe .@ 打开文件夹即可。

## 验收标准

1. 红色三角形在画面中央**绕 Z 轴旋转**,形状大小不变;
2. 程序启动时打印的 @model / view / projection / mvp@ 四个矩阵,前两个不再是单位阵;
3. @projection@ 的最后一行应该是 @[0, 0, -1, 0]@(如果你用的是我讲的那种写法)。

## 对答案(做完再看)

@@@bash
cmake -S . -B build -DHW1_SELFCHECK=ON
cmake --build build -j8
./build/selfcheck     # 会打印预期数值并生成 output/selfcheck.png
@@@

@tests/reference_check.cpp@ 里有参考实现。**建议自己写完再打开**。

## 目录

@@@
hw1/
├── main.cpp                  ← 你要改的(只有两个 TODO)
├── rasterizer.hpp/.cpp       ← 框架:MVP → /w → 视口 → 填充
├── Triangle.hpp              ← 三角形数据
├── CMakeLists.txt
├── run.sh
├── tests/reference_check.cpp ← 参考实现(自检用)
└── output/                   ← 渲染结果
@@@
