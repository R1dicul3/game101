#!/usr/bin/env bash
# GAMES101 HW0 环境探针 —— 在 WSL/Ubuntu 内运行:
#   wsl -e bash /mnt/c/Users/31514/Desktop/game101/tools/env_probe.sh
set +e
echo "== distro / kernel =="
grep PRETTY_NAME /etc/os-release
uname -sr
echo "== cpu / mem =="
echo "nproc=$(nproc)"
free -h | head -2
echo "== packages =="
for p in build-essential cmake libeigen3-dev libopencv-dev libgl1-mesa-dev libglu1-mesa-dev freeglut3-dev libx11-dev x11-apps gdb; do
  v=$(dpkg-query -W -f='${Version}' "$p" 2>/dev/null)
  if [ -z "$v" ]; then echo "MISSING  $p"; else echo "OK       $p $v"; fi
done
echo "== toolchain =="
g++ --version | head -1
cmake --version 2>/dev/null | head -1 || echo "cmake: MISSING"
pkg-config --modversion opencv4 2>/dev/null && echo "(opencv4 via pkg-config)" || echo "opencv4: MISSING"
ls -d /usr/include/eigen3 2>/dev/null || echo "eigen: MISSING"
echo "== gui (WSLg) =="
echo "DISPLAY=[$DISPLAY] WAYLAND=[$WAYLAND_DISPLAY]"
ls /mnt/wslg 2>/dev/null | head -6 || echo "no /mnt/wslg"
echo "== gpu passthrough =="
nvidia-smi -L 2>/dev/null || echo "no nvidia-smi in wsl"
echo "== disk =="
df -h / /mnt/c 2>/dev/null | sed -n '1,3p'
echo "== network / apt =="
timeout 8 curl -sS -o /dev/null -w "archive.ubuntu.com:%{http_code}\n" https://archive.ubuntu.com/ubuntu/ 2>&1 | tail -1
apt-cache policy cmake 2>/dev/null | sed -n '1,4p'
echo "== sudo =="
sudo -n true 2>/dev/null && echo "SUDO_NOPASS" || echo "SUDO_NEEDS_PASSWORD"
