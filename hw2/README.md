# HW2 — 光栅化与 Z-Buffer

三角形投影到屏幕上之后,**怎么变成像素**、**怎么处理遮挡**。这是整个实时渲染管线里最核心的一步。

## 你要实现什么

只有两个函数,都在 `rasterizer.cpp` 中部标注了位置:

| 函数 | 作用 |
|---|---|
| `insideTriangle(x, y, v)` | 判断点是否在三角形内(叉积 / 重心坐标) |
| `rasterize_triangle(t)` | 包围盒遍历 + 重心坐标 + 深度测试 + 写颜色 |

其余全部是框架:`main.cpp`(两个三角形、相机、投影)、`draw()`(MVP + 透视除法 + 视口变换)、`resolve()`(降采样)。

## 跑起来

```bash
bash run.sh          # 1x 渲染 -> output/hw2.png
bash run.sh 2        # 2x2 SSAA -> output/hw2_2x.png
bash run.sh both     # 两张都出,对比锯齿
```

初始状态会输出 `着色像素=0`,因为两个函数还是空的。

## 场景

| 三角形 | 颜色 | z(世界) | 说明 |
|---|---|---|---|
| 红 | (255,0,0) | -2 | 更近 |
| 蓝 | (0,0,255) | -8 | 更远,在红的后面 |

蓝三角比红的大,所以会从红的下方露出一条边 —— **那条边在哪、谁盖住谁,就是 Z-Buffer 正确与否的判据**。

## 验收标准

1. 红三角完整显示,蓝三角只在红的下方露出一条;
2. 程序打印的 `深度范围` 有两个值(约 `0.9754` 与 `0.9886`);
3. `bash run.sh both` 时,2x 版本的三角形边缘明显平滑。

## 对答案(做完再看)

```bash
cmake -S . -B build -DHW2_SELFCHECK=ON
cmake --build build -j8
./build/selfcheck
```

会打印四个探针像素的颜色和深度。

## 目录

```
hw2/
├── main.cpp                  ← 场景与相机(不用改)
├── rasterizer.hpp/.cpp       ← ★ 你的两个函数在这里
├── Triangle.hpp
├── HW2-详解.md               ← 完整思路、公式、错误对照表
├── tests/reference_check.cpp ← 参考实现
└── output/
```

## 详解

数学推导、重心坐标、为什么 NDC z 可以线性插值、常见错误对照表,都在 `HW2-详解.md`。
