#!/usr/bin/env bash
# 提交并推送一次作业进度
#   用法: bash tools/push.sh "HW2: 光栅化完成"
set -euo pipefail
cd "$(dirname "$0")/.."

MSG="${1:-update}"
git add -A
if git diff --cached --quiet; then
    echo "没有改动,跳过提交"
    exit 0
fi
git commit -m "$MSG"
git push
echo
echo "已推送: $MSG"
echo "查看: https://github.com/R1dicul3/game101/commits/main"
