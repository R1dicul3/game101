#!/usr/bin/env bash
# GAMES101 HW1 一键构建 + 运行
#   用法:  bash run.sh            # 编译并渲染 36 帧到 output/
#          bash run.sh 30         # 只渲染 30 度那一帧
#          bash run.sh show       # 开窗口播动画(WSLg 下可用)
set -euo pipefail
cd "$(dirname "$0")"

echo "==> configure"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release > /tmp/hw1_cmake.log 2>&1 || {
    echo "配置失败,日志:"; tail -30 /tmp/hw1_cmake.log; exit 1; }

echo "==> build"
cmake --build build -j"$(nproc)" 2>&1 | tail -5

echo "==> run"
./build/hw1 "$@"

echo "==> 完成。图片在 $(pwd)/output/"
