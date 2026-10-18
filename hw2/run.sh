#!/usr/bin/env bash
# GAMES101 HW2 一键构建 + 运行
#   bash run.sh           # 1x 渲染(基础部分)
#   bash run.sh 2         # 2x2 SSAA(提高部分)
#   bash run.sh both      # 两张都出,方便对比锯齿
set -euo pipefail
cd "$(dirname "$0")"

echo "==> configure"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release > /tmp/hw2_cmake.log 2>&1 || {
    echo "配置失败,日志:"; tail -30 /tmp/hw2_cmake.log; exit 1; }

echo "==> build"
cmake --build build -j"$(nproc)" 2>&1 | tail -5

echo "==> run"
./build/hw2 "$@"

echo "==> 完成。图片在 $(pwd)/output/"
