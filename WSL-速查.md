# WSL 上手速查(GAMES101 用)

> 你的机器:WSL 2.5.7 + **Ubuntu 24.04.1 LTS**,用户名 `linxin`,已经装好 g++ / cmake / Eigen / OpenCV。

---

## 0. WSL 到底是什么

Windows 里跑着的一个**真·Linux**(内核 6.6,不是模拟器),和 Windows 共享 CPU、内存、显卡、硬盘。
它和 Windows 是两个系统,只是**文件互相看得见**:

| Windows 路径 | WSL 里的路径 |
|---|---|
| `C:\Users\31514\Desktop\game101` | `/mnt/c/Users/31514/Desktop/game101` |
| `D:\` | `/mnt/d/` |
| WSL 自己的家目录 | `~` = `/home/linxin` |

**转换规则:** `C:` → `/mnt/c/`,斜杠方向反过来,盘符小写。

> 为什么要用 Linux 做 GAMES101?课程代码按 POSIX 写,Eigen / OpenCV 一条 apt 就装好,WSLg 还能直接弹窗口看图。Windows 原生得自己凑库,吃力。

---

## 1. 怎么打开它

| 方式 | 操作 |
|---|---|
| **Windows Terminal**(推荐) | 开始菜单搜 `Terminal`,点开后在标签栏下拉里选 **Ubuntu** |
| 命令行 | `Win + R` → 输入 `wsl` → 回车 |
| VS Code | 左下角绿色角标 → *Connect to WSL* |

**退出**:输入 `exit` 回车。想彻底关机释放内存:在 **PowerShell** 里跑 `wsl --shutdown`。

常用管理命令(在 PowerShell 里执行,不是在 Linux 里):

```powershell
wsl -l -v          # 列出所有发行版和状态(Running / Stopped)
wsl --shutdown     # 关掉全部 WSL(内存吃多了就来一下)
wsl -d Ubuntu      # 指定进入 Ubuntu
wsl                # 直接进默认发行版
```

---

## 2. 十分钟上手(照着敲)

进 WSL 后,提示符大概长这样:`linxin@机器名:~$`。`~` 表示你当前在**家目录**。

```bash
# ① 我在哪
pwd                      # 打印当前目录

# ② 切到 Windows 桌面的作业目录(注意 /mnt/c 和斜杠方向)
cd /mnt/c/Users/31514/Desktop/game101
ls                       # 看这里有什么

# ③ 进到之前建好的冒烟测试工程
cd hw0_smoke
ls                       # 应该看到 main.cpp  CMakeLists.txt  build/

# ④ 编译 + 运行
cmake --build build -j8  # 增量编译,-j8 = 用 8 个线程并行
./build/smoke            # 运行,打印版本并生成 hw0_smoke.png

# ⑤ 看图(把 Windows 资源管理器叫出来)
explorer.exe .           # 末尾有个点 = 当前目录
```

**应该看到:** 终端打印 OpenCV / Eigen 版本 + `wrote hw0_smoke.png`;资源管理器弹出 `build` 文件夹,里面就是那张图。

### 必备命令小抄

| 想干什么 | 命令 |
|---|---|
| 看当前在哪 | `pwd` |
| 列出文件 | `ls`(`ls -l` 带详情) |
| 进目录 | `cd 目录名`(`cd ..` 上一层,`cd ~` 回家) |
| 看文件内容 | `cat 文件名`(`less 文件名` 可翻页,按 q 退出) |
| 新建目录 | `mkdir 名字` |
| 复制 / 移动 / 删除 | `cp a b` / `mv a b` / `rm 文件`(`rm -rf 目录` 慎用) |
| 输出重定向 | `命令 > log.txt 2>&1`(stdout + stderr 都进日志) |
| 找命令在哪 | `which cmake` |
| 装软件 | `sudo apt update` 然后 `sudo apt install -y 包名` |
| 清屏 | `clear`(或 Ctrl+L) |
| 中断正在跑的程序 | **Ctrl+C** |
| 补全路径 | **Tab 键**(多按几次会列出来) |
| 翻历史命令 | **↑ / ↓** |

---

## 3. 怎么改代码(三种方式)

### 方式 A:VS Code 连接 WSL(强烈推荐)

1. VS Code 里安装扩展 **WSL**(ID: `ms-vscode-remote.remote-wsl`)
2. 在 WSL 终端里进到代码目录,敲 `code .`
3. 会弹出一个连着 WSL 的 VS Code 窗口,左下角显示 `WSL: Ubuntu`
4. 之后编辑、保存、开终端(Ctrl + 反引号)、调试全在这个窗口里,**不用来回切**

> 好处:文件真的在 Linux 侧,编译器也是 Linux 的,不会出现"Windows 版 VS Code + Linux 编译器"的混线问题。

### 方式 B:用 Windows 的编辑器改 /mnt/c 下的文件

工作区就在 `C:\Users\31514\Desktop\game101`,可以直接在 Windows 的 VS Code 里改,回到 WSL 里 `cmake --build build` 重新编译。
**简单、够用**,唯一注意:**保存成 UTF-8、LF 换行**。

### 方式 C:命令行编辑器

`nano main.cpp`(底部有快捷键提示,^O 保存、^X 退出)。不熟就别碰 vim。

---

## 4. 图形界面:WSLg 是白送的

你这台机器 **WSLg 已经能用**(`DISPLAY=:0`),意思是 `cv::imshow()` 这类窗口**会像 Windows 程序一样弹出来** —— 不用装 VcXsrv,不用配 DISPLAY。

| 方式 | 做法 | 适用 |
|---|---|---|
| 直接弹窗 | 代码里 `cv::imshow(); cv::waitKey(0);` | 调试最快,但是**阻塞**的 |
| 存图 + 资源管理器 | `cv::imwrite("out.png", img)` 之后 `explorer.exe .` | 交作业截图首选 |
| 本机看图工具 | Windows 里双击 PNG | 同上 |

> 提醒:`imshow` 会卡住程序直到你按键。批量出图(比如旋转 36 帧)时别用,直接 `imwrite` 成序列。

---

## 5. 踩坑清单

1. **大小写**:Windows 不区分,Linux **区分**!`Main.cpp` 和 `main.cpp` 是两个文件,CMake 里写错就报"找不到源文件"。
2. **`/mnt/c` 比 Linux 原生目录慢 2–5 倍**:小项目无所谓;真嫌慢就在家目录 `~` 里开发。
3. **换行符**:Windows 的 CRLF 进 Linux 会让脚本报 `bad interpreter`。VS Code 右下角把行尾改成 **LF**。
4. **文件权限**:`/mnt/c` 里新建的脚本默认没有执行权限,要用 `bash 脚本.sh` 而不是 `./脚本.sh`。
5. **内存**:WSL 默认最多吃一半物理内存(你这里分了 15 GiB)。`wsl --shutdown` 全部还回去。想限额就在 `C:\Users\31514\.wslconfig` 写 `[wsl2]` 下的 `memory=8GB`。
6. **代理**:你这台机器有本地代理,Docker 提示过"localhost 代理未镜像到 WSL"。WSL 里访问 GitHub 可能超时,apt 一般没事。
7. **`sudo` 要密码**:每次装包都要输 Windows 账户密码;**输入时屏幕上不显示**,不是卡了。
8. **两个终端别搞混**:`PS C:\...>` 是 **PowerShell**(Windows),`linxin@...:~$` 是 **Linux**。`dir` 在 Linux 里不存在,用 `ls`。

---

## 6. GAMES101 的标准工作流

```bash
cd /mnt/c/Users/31514/Desktop/game101/hw1      # 进作业目录
code .                                         # 用 VS Code(WSL 模式)打开,改 main.cpp
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release  # 首次配置(只做一次)
cmake --build build -j8                         # 编译
./build/hw1                                     # 运行出图
explorer.exe .                                  # 打开文件夹,交作业截图
```

---

*速查表由 Mocha 整理,随时可以让我补充或改写成更适合你习惯的版本。*
