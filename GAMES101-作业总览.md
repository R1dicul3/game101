# GAMES101 现代计算机图形学入门 —— 作业总览

> 课程: GAMES101《现代计算机图形学入门》(闫令琪,Lingqi Yan, UCSB)
> 公开版本共 **8 次作业**:HW0 ~ HW7(第 8 次通常是「大作业 / 比赛」,视开课年份而定)
> 本文档整理自公开的作业说明与课程讲义,用来做进度管理与复习索引。
> 具体提交要求、截止时间、加分项请以**你自己那份作业 PDF / 课程通知**为准,本文档不替代原文。

---

## 一、速查总表

| # | 作业主题 | 对应 Lecture | 难度 | 核心技术点 | 主要输出 |
|---|---------|-------------|------|-----------|---------|
| HW0 | 环境准备 | Preliminaries | ⭐ | Ubuntu/WSL + Eigen + OpenCV + CMake | 编译成功 + 一张输出图 |
| HW1 | 变换与投影 (MVP) | 3–4 | ⭐⭐ | 旋转矩阵、正交/透视投影、视口变换 | 绕 Z 轴旋转的图 + 三个矩阵 |
| HW2 | 光栅化 | 5–6 | ⭐⭐⭐ | 三角形光栅化、Z-Buffer、MSAA/SSAA 反走样 | 有遮挡关系的三角形 + 抗锯齿对比 |
| HW3 | 着色 | 7–10 | ⭐⭐⭐⭐ | Blinn-Phong、纹理映射、Bump / Displacement | 四种着色器的渲染结果 |
| HW4 | 曲线 (Bézier) | 11 | ⭐⭐ | de Casteljau 递归、反走样 | 连续平滑的贝塞尔曲线 |
| HW5 | 光线追踪(基础) | 13–14 | ⭐⭐⭐⭐ | 光线生成、Möller–Trumbore、Whitted 递归 | 带反射/折射的球体场景 |
| HW6 | 加速结构 (BVH) | 13 | ⭐⭐⭐⭐ | AABB 求交、BVH 构建、SAH | 与朴素求交同图 + 大幅加速比 |
| HW7 | 路径追踪 | 15–16 | ⭐⭐⭐⭐⭐ | 蒙特卡洛积分、面光源采样、RR、多线程 | 无噪点的 Cornell Box |

---

## 二、逐次作业详情

### HW0 — 环境准备

**目标**:把实验环境跑通,不改任何图形学代码。

- 装好 **Ubuntu**(虚拟机 / 双系统 / WSL2 均可)或在本机配置好工具链;
- 安装 **Eigen**(建议 3.3.x,课程代码大量依赖它的向量/矩阵类型)、**OpenCV**(建议 4.x)、**CMake**(≥ 3.10)、**g++**;
- 编译 Assignment0,运行后得到一张默认生成的图片(验证 Eigen / OpenCV 链接正常);
- 需要图形界面时,记得配好 X11 / XQuartz / VcXsrv 之类的显示转发。

**常见坑**
- Ubuntu 源里的 `libeigen3-dev` 版本过旧,部分作业要求手动下载 Eigen 3.3.7 并把头文件路径写进 CMakeLists;
- OpenCV 装了多个版本时 `find_package` 会找错,注意 `OpenCV_DIR`;
- Windows 原生编译(MSVC)能过,但课程代码假设了 POSIX 路径,用 WSL 更省事。

**验收**:截图证明编译 0 error + 能弹出一张图。

---

### HW1 — 变换与投影

**目标**:亲手实现 MVP 流水线里的 M 和 P。

必做函数:
- `get_model_matrix(float rotation_angle)` —— 绕 **Z** 轴旋转的 4×4 齐次矩阵;
- `get_projection_matrix(float eye_fov, float aspect_ratio, float zNear, float zFar)` —— 透视投影,需要先把它拆成「透视压缩到正交盒」+「正交投影」两步(即 Lecture 4 的 `M_persp→ortho` 与 `M_ortho`);
- 在 `main.cpp` 里把这两个矩阵乘进 `MVP` 并让图像转起来(通常是按角度逐帧输出或输出一组图)。

**公式复习**
```
M_persp→ortho = [ n, 0, 0, 0 ]
                [ 0, n, 0, 0 ]
                [ 0, 0, n+f, -nf ]
                [ 0, 0, 1, 0 ]
```
注意 n、f 的符号约定(Yan 的课上 zNear/zFar 是正数,而视图空间看向 **−z**,符号搞反是最高频 bug)。

**验收**:图像绕 Z 轴旋转序列正确;三个矩阵打印出来符合预期。

---

### HW2 — 光栅化

**目标**:从零实现「三角形 → 像素」这一步,并理解抗锯齿。

必做:
- `rasterize_triangle(const Triangle& t)` —— 求包围盒、逐像素判断、写颜色与深度;
- `insideTriangle(x, y, v)` —— 用重心坐标或三次叉积判断点是否在三角形内;
- **Z-Buffer**:维护 `depth_buf`,只有更近的片元才写入颜色;
- **反走样**:课程要求实现 **2×2 超采样(MSAA)**,并对每个子采样点分别做深度测试,再取平均;作业还会要求打开更高质量的采样做对比。

**常见坑**
- 屏幕坐标 `x, y` 是**像素整数坐标**,判断三角形覆盖要取像素中心 (`x+0.5, y+0.5`);
- 深度 `z` 是**越大越远**(视图空间 −z),所以 z-buffer 初始化为 `+∞`,更新条件是 `z < depth_buf`;
- 重心坐标用 float,别用 int 截断,否则边界像素抖动;
- SSAA/MSAA 别忘记 z-buffer 也要按子采样分别存,否则边缘会有「深度穿透」。

**验收**:两个 / 多个三角形前后遮挡关系正确;抗锯齿前后边缘对比明显。

---

### HW3 — 着色

**目标**:实现四种片元着色器,理解「光照模型 + 纹理 + 法线扰动」。

必做:
- `phong_fragment_shader` —— Blinn-Phong,环境光 + 漫反射 + 高光,half-vector 代替反射向量;
- `texture_fragment_shader` —— 用 `uv` 采样贴图颜色(注意 `getUV` 与纹理坐标的 v 方向翻转);
- `bump_fragment_shader` —— 凹凸映射,从纹理的 **高度场差分**求扰动后法线,TBN 变换回世界空间;
- `displacement_fragment_shader` —— 位移映射,不仅改法线还改**顶点位置**(沿法线偏移);
- 顺带要正确处理 **法线变换**:法线要用模型矩阵的**逆转置**变换。

**关键公式**(bump,切空间下)
```
dU = kh * kn * (H(x+1,y) - H(x,y))
dV = kh * kn * (H(x,y+1) - H(x,y))
ln = normalize(vec3(-dU, -dV, 1))
n  = TBN * ln
```

**常见坑**
- 只给 (u,v) 求法线就完了 —— 忘了算 TBN 的切线方向;
- 纹理采样越界 / 重复平铺方式错误;
- Blinn-Phong 里高光指数太小导致整片发亮;光源方向未归一化。

**验收**:四张图分别对应 Phong / Texture / Bump / Displacement,且 bump 的明暗细节能对上高度场。

---

### HW4 — 曲线(Bézier)

**目标**:实现 de Casteljau 递归,画出光滑曲线,顺便练反走样。

必做:
- `recursive_bezier(control_points, t)` —— 每一层线性插值,直到只剩一个点;
- `bezier(control_points, ...)` —— 在 [0,1] 上均匀取 t,把点连成折线并画出来;
- 与给定控制点的 `naive_bezier` 公式结果对比,验证正确性;
- 实现**反走样**:在像素中心多次采样(或在边缘做加权),消除阶梯;
- 提高部分:实现 **morphological antialiasing**(形态学抗锯齿),或对曲线做粗细/端点处理。

**常见坑**
- 控制点数量可变(`cv::Point2f` 的 vector),递归时容易写死 4 个点;
- t 的步长太小导致点过密、太大导致折线明显 —— 通常取 `1/100` 起步;
- OpenCV 的 `circle`/`line` 默认不做抗锯齿,要开 `LINE_AA` 或自己采样。

**验收**:曲线穿过首末控制点、形状与 naive 公式一致、边缘平滑无锯齿。

---

### HW5 — 光线追踪(基础)

**目标**:手写一条完整的光线追踪链路。

必做:
- 光线生成:按 `scene.width / height` 遍历像素,由相机 `eye_fov / aspect_ratio` 反推方向,经 `cameraToWorld` 变换成世界空间射线;
- `Render()` 中为每像素生成主光线并调用 `castRay(scene, ray, depth)`;
- `castRay`:`trace` 找到最近交点并递归处理反射、折射(Whitted 式),根据材质 `REFLECTION / REFRACTION / DIFFUSE` 分支;
- `rayTriangleIntersect(...)` —— 用 **Möller–Trumbore** 求三角形交点与重心坐标;
- 交点距离要与 `t_tnear` 比较,避免自交(Shadow acne),常规做法是把 tnear 设成 `0.0001`。

**Möller–Trumbore**
```
E1 = v1 - v0, E2 = v2 - v0, S = O - v0, S1 = cross(dir, E2), S2 = cross(S, E1)
t = dot(S2, E2) / dot(S1, E1)   u = dot(S, S1) / det   v = dot(dir, S2) / det
```

**验收**:画面里球体/三角形有正确的反射与折射;渲染结果随 spp、maxdepth 变化合理;两张不同 maxdepth 的图能看出差别。

---

### HW6 — 加速结构(BVH)

**目标**:把 O(n) 的求交降到 O(log n),这是渲染器真正能跑起来的前提。

必做:
- `BoundingBox::IntersectP(ray)` —— 用 **slab 方法**做 ray–AABB 求交;
- `BVHBuildNode*` 构建:`recursiveBuild` 递归切分,取对象包围盒中心的某一轴排序、从中位数劈开;
- `getIntersection(BVHBuildNode* node, const Ray& ray)` —— 先测包围盒,命中再递归左右子节点,在叶子节点里逐个求交;
- 提供 `visualize / naive` 两种模式对比:**同一张图 + 时间差**;
- 提高部分:用 **SAH(Surface Area Heuristic)** 替代中位数划分,进一步降低求交时间;还可以做更紧凑的节点布局。

**常见坑**
- 包围盒的 tNear/tFar 计算里除 0 导致 NaN —— 需要处理方向分量为 0 的情况;
- 叶子节点里求交后忘记取最近的 t;
- 建树时对象数组的**下标对齐**(不是复制对象)弄错,导致包围盒与实际物体错位。

**验收**:BVH 渲染结果与 naive 一致(逐像素几乎相同),耗时显著下降(通常一个数量级以上)。

---

### HW7 — 路径追踪

**目标**:从 Whitted 走向真正的全局光照,并让渲染器跑到可用的速度。

必做:
- 把 `castRay` 改写成**路径追踪**形式:每次反弹按 BRDF 与余弦项做重要性采样,累加 `Le` 后继续追;
- `SampleLight` —— 对**面光源**做采样(面积 × 立体角 / 概率密度),配合直接光照项;
- 正确计算 `pdf`、`f_r`、`cos` 的乘积除以 pdf,否则画面整体偏亮/偏暗;
- 提高部分:
  - **俄罗斯轮盘赌(Russian Roulette)**:按存活概率 `P` 截断路径,期望值不变但能加速;
  - **多线程**:用 `std::thread` 或 OpenMP 按像素/行切分,注意共享的 `framebuffer` 写入要加锁或分区;
  - 更高 spp 得到更干净的画面。

**常见坑**
- 忘记除以 `pdf`,导致结果随采样分布而变;
- 采样光源时没考虑「光源是否被遮挡」,出现漏光;
- 多线程里直接 `++progress` 不加锁,统计数据错乱;
- 反射/折射方向没归一化,能量不守恒。

**验收**:Cornell Box(或给定场景)无明显噪点、阴影软硬合理、不同 spp 收敛趋势正确,并且多线程/轮盘赌带来可测量的加速。

---

## 三、通用要点(几乎每次作业都踩)

1. **矩阵与坐标系约定**:课程统一用**列向量右乘**(`M * v`),视图空间看向 `-z`,zNear/zFar 是正距离。混用行主序或右手/左手系是最普遍的返工原因。
2. **法线变换用逆转置**:`normalMatrix = model.inverse().transpose()`,直接用 model 会让非等比缩放的法线出错。
3. **齐次坐标除法**:透视投影后必须做 `x/w, y/w, z/w`,忘了就得到一张全白或全黑的图。
4. **浮点精度**:z-buffer、射线求交的 tnear 都需要 epsilon,否则出现 z-fighting 与自交黑点。
5. **提交物**:通常包含①源码(可编译,别提交 build 目录)②关键结果图③一份说明做了哪些部分、哪些是加分项的报告。报告里写清楚自己实现的思路,比贴代码更拿分。
6. **性能别忽视**:HW5/HW6/HW7 都是「跑完」比「写对」更难,先用低分辨率低 spp 调通,再上高参数出图。

---

## 四、进度追踪

| 作业 | 状态 | 完成日期 | 备注 |
|------|------|---------|------|
| HW0 环境准备 | ✅ 已完成 | 2026-10-18 | Eigen 3.4 + OpenCV 4.6 + CMake 3.28 编译/链接/出图全通,见 `hw0_smoke/` |
| HW1 变换与投影 | ✅ 已完成 | 2026-10-19 | 两函数均通过:z 行正确,z_ndc 近/中/远 = -1 / 0.9754 / +1;36 帧与参考渲染逐像素一致 |
| HW2 光栅化 | ⬜ 未开始 | | |
| HW3 着色 | ⬜ 未开始 | | |
| HW4 贝塞尔曲线 | ⬜ 未开始 | | |
| HW5 光线追踪 | ⬜ 未开始 | | |
| HW6 BVH 加速 | ⬜ 未开始 | | |
| HW7 路径追踪 | ⬜ 未开始 | | |

> 状态可用 ⬜ 未开始 / 🟡 进行中 / ✅ 已完成 标记。

---

## 五、本机环境核查(2026-10-18 实测)

### 硬件 / 系统

| 项目 | 实测值 |
|------|--------|
| 操作系统 | Windows 11,build 10.0.26200.9711(24H2/25H2 分支) |
| CPU | 13th Gen Intel Core i9-13980HX,32 线程 |
| GPU | NVIDIA GeForce RTX 4060 Laptop,8188 MiB,驱动 617.42 |
| 磁盘 | C: 121 GB 可用 / D: 85.6 GB 可用 |
| WSL | WSL **2.5.7.0**,内核 6.6.87.1-1,**WSLg 1.0.66** |

### Windows 侧工具链

| 工具 | 状态 |
|------|------|
| Visual Studio 2026 Community (18.10.2) | ✅ 含 MSVC |
| CMake | ✅ 4.3.1(随 VS 捆绑,未加进 PATH) |
| Git | ✅ 2.43.0(`D:\Git`) |
| Python | ✅ 3.13.5,cv2 4.10.0 / numpy 2.2.5 |
| VS Code | ✅ `D:\Microsoft VS Code` |
| MinGW g++ | ⚠️ 6.3.0 —— **太旧,不要用来做 GAMES101** |
| vcpkg | ✅ `C:\vcpkg`,但只装了 OpenGL / GLEW 的 x86-mingw-static,没有 OpenCV / Eigen |

### WSL 侧(Ubuntu 24.04.1 LTS,用户 `linxin`,32 vCPU / 15 GiB,根分区 954 GB 可用)

| 组件 | 状态 |
|------|------|
| g++ 13.3.0 / gcc 13.2 | ✅ 已有 |
| git 2.43 / python3 3.12 | ✅ 已有 |
| **cmake** | ❌ 缺失(apt 候选项 3.28.3,已可安装) |
| **Eigen3** | ❌ 缺失(`/usr/include/eigen3` 不存在) |
| **OpenCV** | ❌ 缺失(无 `opencv4` pkg-config) |
| build-essential / GL 头文件 / X11 / gdb | ❌ 缺失 |
| WSLg 图形界面 | ✅ 正常(`DISPLAY=:0`,`WAYLAND_DISPLAY=wayland-0`)→ **不用装 VcXsrv** |
| GPU 直通 | ✅ WSL 内 `nvidia-smi` 可见 RTX 4060 |
| 网络 | ✅ archive.ubuntu.com 可达(HTTP 200);⚠️ GitHub 下载偶发超时 |
| sudo | ⚠️ **需要密码**,自动安装需要你自己输一次 |

### 结论与建议路线

1. **走 WSL(Ubuntu 24.04)** 做 HW0–HW7:课程代码按 POSIX 编写,Eigen / OpenCV 的 apt 包现成,WSLg 能直接弹窗看图,不必折腾虚拟机或 VcXsrv。
2. Windows 原生(MSVC + VS 自带 CMake)也能编译,但要自己找 Eigen / OpenCV 的预编译包,性价比低,只建议当备用。
> **2026-10-18 更新:环境已装好并验证通过。** 实测 `g++ 13.3 / cmake 3.28.3 / Eigen 3.4.0 / OpenCV 4.6.0`,
> `hw0_smoke/` 这个最小工程用 CMake 完成 Eigen + OpenCV 联合编译、链接并成功输出 700×700 PNG,即 HW0 通过。

3. 当时补齐环境用的命令(留档):

```bash
sudo apt update && sudo apt install -y build-essential cmake libeigen3-dev libopencv-dev \
  libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev x11-apps gdb
```

4. 版本提示:Ubuntu 24.04 的 `libopencv-dev` 是 4.6.x、`libeigen3-dev` 是 3.4.x,均高于课程所用版本(OpenCV 4 / Eigen 3.3.7),接口兼容;若某次作业报 Eigen 相关编译错误,再手动降级到 3.3.7。
5. 环境探针脚本:`tools/env_probe.sh`,随时可在 WSL 里重跑核对:`wsl -e bash /mnt/c/Users/31514/Desktop/game101/tools/env_probe.sh`

---

## 六、参考资料

- 课程主页与讲义:GAMES101 官网(每年更新的 Lecture 视频 + 课件 PDF)
- 数学参考:《Linear Algebra》/ Scratchapixel 的 Transform 章节
- 光线追踪参考:《Ray Tracing in One Weekend》系列(思路相通,可对照 HW5–HW7)
- BVH / SAH:PBRT 第 4 章 Accelerations Structures
- 路径追踪:PBRT 第 13–14 章 Monte Carlo 与 Light Transport

---

*本文件由 Mocha 生成于 C:\Users\31514\Desktop\game101。若课程实际作业编号或要求与本文档不同,以课程 PDF 为准,并告诉我来更新此文档。*
