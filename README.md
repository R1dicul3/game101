# GAMES101 学习记录

GAMES101《现代计算机图形学入门》(闫令琪)的作业实现与学习笔记,记录从环境搭建到路径追踪的完整过程。

- 开发环境:**WSL2 + Ubuntu 24.04**(C++17 / CMake / Eigen 3.4 / OpenCV 4.6)
- 每完成一次作业提交一次,commit 记录即学习轨迹

## 进度

| # | 主题 | 状态 | 完成日期 | 产物 |
|---|------|------|---------|------|
| HW0 | 环境准备 | ✅ | 2026-10-18 | `hw0_smoke/` |
| HW1 | 变换与投影 | ✅ | 2026-10-19 | `hw1/` |
| HW2 | 光栅化 | ⬜ | | |
| HW3 | 着色 | ⬜ | | |
| HW4 | 贝塞尔曲线 | ⬜ | | |
| HW5 | 光线追踪 | ⬜ | | |
| HW6 | BVH 加速 | ⬜ | | |
| HW7 | 路径追踪 | ⬜ | | |

## 目录

`@
game101/
├── GAMES101-作业总览.md      # 8 次作业的完整清单、知识点、常见坑
├── WSL-速查.md               # WSL/Ubuntu 上手手册
├── hw0_smoke/                # HW0:Eigen + OpenCV + CMake 联合编译验证
├── hw1/                      # HW1:变换与投影
│   ├── main.cpp              #   两个待实现函数(已实现)
│   ├── rasterizer.*          #   框架:MVP -> 透视除法 -> 视口 -> 填充
│   ├── HW1-详解.md           #   数学推导 + 数值验证 + 症状对照表
│   ├── demos/                #   Eigen 语法小样
│   ├── tests/                #   参考实现(自检用)
│   └── output/               #   渲染结果
└── tools/env_probe.sh        # WSL 环境探针
`@

## 环境

`@bash
sudo apt install -y build-essential cmake libeigen3-dev libopencv-dev \
  libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev x11-apps gdb
`@

| 组件 | 版本 |
|---|---|
| Ubuntu | 24.04.1 LTS (WSL2) |
| g++ | 13.3.0 |
| CMake | 3.28.3 |
| Eigen | 3.4.0 |
| OpenCV | 4.6.0 |
| NVIDIA | RTX 4060 Laptop 8GB(WSL GPU 直通可用) |

## 运行

`@bash
cd hw1
bash run.sh          # 配置 + 编译 + 渲染 36 帧到 output/
bash run.sh 45       # 只渲染 45 度
bash run.sh show     # 开窗口播动画(WSLg)
`@

## 提交流程

每完成一次作业提交一次,commit 就是学习轨迹:

```bash
bash hw1/run.sh                      # 先跑通
bash tools/push.sh "HW2: 光栅化完成"   # 再提交推送
```

## 作业记录

### HW0 —— 环境准备

验证工具链能联合编译并出图。`hw0_smoke/main.cpp` 用 Eigen 算旋转矩阵、驱动 OpenCV 画多边形,输出 700x700 PNG。

**结果:** g++ 13.3 / CMake 3.28 / Eigen 3.4 / OpenCV 4.6 全部打通,WSLg 图形界面可用。

### HW1 —— 变换与投影

实现 `get_model_matrix`(绕 Z 轴旋转)与 `get_projection_matrix`(透视投影),让三角形绕 Z 轴旋转。

**关键收获:**
1. 透视投影的最后一行 `[0,0,-1,0]` 是灵魂 —— 它让 `w = -z`,除法才产生"近大远小"
2. 两段式(透视压缩 x 正交投影)手抄极容易把常数项符号搞反;错的时候 `x/y` 全对、画面看着正常,只有 `z_ndc` 是坏的,HW1 看不出来、HW2 的 Z-Buffer 会立刻崩
3. 三种深度自检:近平面 `z_ndc = -1`、远平面 `z_ndc = +1`、顶点 `z_ndc = 0.9754`

**结果:** 36 帧面积恒定(绕 Z 轴旋转不改变面积),与参考渲染逐像素一致。

详见 `hw1/HW1-详解.md`。
