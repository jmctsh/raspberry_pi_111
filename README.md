# 赛博木鱼（Raspberry Pi 5，C++）

这是一个在树莓派 5 上运行的 C++ 终端程序，启动后显示“赛博木鱼”的字符界面。通过连接到 40Pin 排针上的一个按键，每按一次按钮，界面上会出现“功德+1”的动画，并在木鱼下方累计统计功德总数。

## 功能概览
- 终端 ASCII UI 显示赛博木鱼
- 监听物理按键（下降沿触发），每次按下增加功德
- 键盘模拟：空格或回车也可增加功德；按 `q` 退出
- 简单去抖处理（软件 150ms）

## 硬件连接（按钮接线详解）
本程序默认使用 BCM GPIO17（物理引脚 11）作为按键输入，并启用内置上拉（Pull-up）。推荐接法如下：

- 选择一个轻触按钮（常开，按下导通）
- 将按钮一端接到物理引脚 11（BCM GPIO17）
- 将按钮另一端接到 GND（推荐物理引脚 9，或 6/14/20/25/30/34/39 之一）

这样接法的逻辑为：
- 平时由于上拉，GPIO 处于高电平；
- 按下按钮后，GPIO 被接到地（GND），产生“下降沿”事件；
- 程序捕获事件后触发“功德+1”。

注意事项：
- 树莓派 GPIO 只能承受 3.3V 逻辑电平，禁止将 5V 直接接到 GPIO！
- 如果你不使用程序的内置上拉（默认启用），也可以外接一个 10kΩ 上拉电阻到 3.3V，但一般无需外接。
- 接线时尽量在断电状态下进行，或确保不短路。

引脚速查（常用）：
- 物理引脚 1：3.3V
- 物理引脚 6/9/14/20/25/30/34/39：GND（地）
- 物理引脚 11：BCM GPIO17（默认按钮输入）

## 软件安装与编译
在树莓派 5（Raspberry Pi OS Bookworm）上，安装依赖并编译：

1. 安装依赖
   ```bash
   sudo apt update
   sudo apt install -y build-essential cmake libgpiod-dev libncurses-dev
   ```

2. 编译
   ```bash
   mkdir -p build
   cd build
   cmake ..
   make -j
   ```

3. 运行
   ```bash
   # 可选指定 GPIO 号码（BCM 编号），未指定默认 17
   ./cyber_muyu 17
   ```

如果运行提示权限不足，尝试使用 sudo：
```bash
sudo ./cyber_muyu 17
```

说明：
- 本程序会自动在多个 gpiochip 上尝试打开你指定的 GPIO 号码（适配树莓派 5 的 RP1 设备划分），无需关心具体 /dev/gpiochipX 的编号。
- 若无法捕获按钮事件，仍可用键盘空格/回车进行功能模拟以验证 UI。

## 使用方法与交互
- 按下物理按钮：显示“功德+1”动画，并累计功德数
- 键盘空格/回车：模拟增加功德
- 键盘 `q`：退出程序

## 故障排查
- 权限问题：不具备访问 /dev/gpiochipX 的权限时会失败，建议使用 sudo 运行，或配置 udev 规则开放权限。
- 接线方向：确保按钮一端接到 BCM GPIO17（物理 11），另一端接地（任意 GND 引脚）。
- 反应过于灵敏或误触发：可适当增大软件去抖时间（源码中为 150ms），或在硬件端增加 RC 去抖。

## 自定义 GPIO
运行时传入 BCM GPIO 号即可，例如使用 GPIO27（物理引脚 13）：
```bash
./cyber_muyu 27
```
同样将按钮一端接到对应 GPIO（物理引脚 13），末端接到任意 GND。

## 代码托管与同步（Windows 上传 + 树莓派拉取/更新）

以下步骤基于 HTTPS 方式，适合多数用户。若你启用了双重验证，推送/拉取时会提示使用 GitHub 账号与个人访问令牌（PAT）。

### 在 Windows 上把本项目上传到 GitHub（首次）
1. 安装 Git（未安装可到 Git for Windows 官方页面安装）。
2. 打开 PowerShell，进入项目目录：
   ```powershell
   cd d:\raspberry_pi
   ```
3. 配置全局用户名/邮箱（首次使用 Git 需要）：
   ```powershell
   git config --global user.name "你的名字"
   git config --global user.email "你的邮箱@example.com"
   ```
4. 初始化仓库、提交代码、设置远程并推送到主分支（main）：
   ```powershell
   git init
   git add .
   git commit -m "Initial commit"
   git branch -M main
   git remote add origin https://github.com/jmctsh/raspberry_pi_111.git
   git push -u origin main
   ```
   提示登录时，按指引完成 GitHub 认证（建议使用 Git Credential Manager 进行保存）。

可选建议：将构建目录排除在版本控制之外（本项目默认未生成 build 目录）。若你本地使用 build/ 进行编译，可创建 .gitignore 并写入：
```
build/
```

### 在树莓派上拉取（首次）
1. 安装 Git：
   ```bash
   sudo apt update
   sudo apt install -y git
   ```
2. 克隆仓库到你的家目录：
   ```bash
   cd ~
   git clone https://github.com/jmctsh/raspberry_pi_111.git
   cd raspberry_pi_111
   ```
3. 编译与运行请参考上文“软件安装与编译”章节中的步骤（安装依赖、cmake 构建、运行）。

### 后续代码更新时如何在树莓派上拉取
- 若你没有在树莓派上修改过代码（仅用于运行），直接在仓库目录执行：
  ```bash
  cd ~/raspberry_pi_111
  git pull
  ```
- 若你在树莓派上也做了本地修改，推荐先保存或暂存再拉取：
  ```bash
  # 暂存当前改动
  git stash
  # 拉取最新代码
  git pull
  # 恢复暂存（可能需要解决冲突）
  git stash pop
  ```
- 若你只想强制以远程代码覆盖本地（会丢弃本地未提交的改动）：
  ```bash
  git fetch origin
  git reset --hard origin/main
  ```

### 常用检查命令
- 查看当前远程：
  ```bash
  git remote -v
  ```
- 查看当前分支：
  ```bash
  git branch
  ```
- 查看拉取/推送状态：
  ```bash
  git status
  ```

> 提醒：如果拉取或推送提示认证问题，使用你的 GitHub 账号与个人访问令牌（PAT）完成登录即可。


## GPIO 常见检查与诊断命令

说明：以下命令用于确认某个 BCM 引脚是否空闲、监听按钮事件、查看模式与上下拉。

前置安装（如未安装）：
- sudo apt install -y gpiod
- sudo apt install -y raspi-gpio

快速映射与占用检查：
- 映射 BCM → 芯片与线号（以 BCM17 为例）：
  - gpiofind GPIO17
  - 典型输出：gpiochip0 17（表示在 gpiochip0 上的第 17 号线）
- 查看整芯片信息并定位第 17 号线：
  - gpioinfo gpiochip0
  - 只看第 17 号线（可选）：
    - gpioinfo gpiochip0 | sed -n 's/^\s*line\s\+17.*/&/p'
- 判断规则：
  - consumer 字段为空或显示 unused → 该线空闲，可直接使用
  - consumer 显示 gpio-keys 等非空名称 → 被内核/驱动占用，建议改用其它 GPIO（如 23/24/25），或改用 /dev/input（evdev）方案读取按钮

监听按钮事件（边沿触发）：
- gpiomon gpiochip0 17
- 操作：按下/松开按钮后应看到时间戳与边沿（rising/falling）。若报 Resource busy 或 Operation not permitted，说明被占用或权限不足；可尝试 sudo 运行或更换 GPIO。

读取当前电平（0/1）：
- gpioget gpiochip0 17

查看 BCM 模式与上下拉：
- raspi-gpio get 17
- 输出包含 func=INPUT/OUTPUT、pull=UP/DOWN/OFF 等，便于确认是否为输入与拉高/拉低。

查看头针脚整体布局信息：
- pinout

与程序配合运行：
- ./cyber_muyu 17
- 若权限不足：sudo ./cyber_muyu 17