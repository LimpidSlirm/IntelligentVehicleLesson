# IntelligentVehicleLesson

TC264 智能小车课程工程，包含双轮电机控制、编码器速度反馈、PID 控制、按键输入和裸机应用层代码。

This repository contains the course project for a TC264 intelligent vehicle. It includes dual-wheel motor control, encoder feedback, PID speed control, key input, and bare-metal application code.

## 项目概览 | Overview

本项目采用裸机周期任务和模块化接口，不依赖 RTOS。控制链路大致如下：

```text
按键或应用命令
    ↓
目标速度
    ↓
编码器反馈 → 左右轮速度 PID
    ↓
电机 PWM 和方向输出
```

The project uses a modular bare-metal design with periodic tasks and does not depend on an RTOS:

```text
Keys or application commands
    ↓
Target speed
    ↓
Encoder feedback → Left/right speed PID
    ↓
Motor PWM and direction output
```

## 硬件信息 | Hardware

当前工程面向 Infineon AURIX TC264 平台，使用双轮直流电机、增量编码器和电机驱动器。

The project targets the Infineon AURIX TC264 platform with two DC motors, incremental encoders, and a motor driver.

已验证的硬件映射如下：

The verified hardware mapping is:

| 功能 / Function | 映射 / Mapping |
|---|---|
| 左轮电机 / Left motor | P02.4 / P02.5 |
| 右轮电机 / Right motor | P02.6 / P02.7 |
| 左轮编码器 / Left encoder | TIM5，P10.3 计数，P10.1 方向 |
| 右轮编码器 / Right encoder | TIM6，P20.3 计数，P20.0 方向 |
| KEY3 | P11.2 |
| KEY4 | P11.3 |
| 舵机 PWM / Servo PWM | P33.9，50 Hz |

电机接口使用 A/B 双输入 H 桥逻辑。修改硬件映射前，请先核对驱动板、原理图和实际接线。

The motor interface uses dual-input A/B H-bridge logic. Verify the driver board, schematic, and physical wiring before changing the mapping.

## 目录结构 | Directory Structure

```text
code/
    app.c/.h                 应用逻辑 / Application logic
    motor.c/.h               电机驱动 / Motor driver
    speed.c/.h               速度控制 / Speed control
    chassis.c/.h             底盘接口 / Chassis interface
    servo.c/.h               舵机接口 / Servo interface
    pid.c/.h                 PID 算法 / PID algorithm
    key.c/.h                 按键模块 / Key module
    led.c/.h                 LED 模块 / LED module
    camera.c/.h              摄像头采集与阈值 / Camera capture and threshold
    otsu.c/.h                Otsu 阈值算法 / Otsu threshold algorithm
    motion.h                 运动接口 / Motion interface

user/
    cpu0_main.c/.h           CPU0 入口 / CPU0 entry
    cpu1_main.c/.h           CPU1 入口 / CPU1 entry
    isr.c/.h                 中断服务 / Interrupt service routines
    isr_config.h             中断配置 / Interrupt configuration

build_headless.bat           无界面编译 / Headless build
flash.bat                    编译并烧录 / Build and flash
```

第三方库和芯片支持文件不在本仓库的课程代码范围内。不要直接修改第三方库文件。

Third-party libraries and chip support files are outside the course code scope. Do not modify third-party library files directly.

## 编译 | Build

在 Windows 命令提示符或 PowerShell 中进入仓库根目录，执行：

From a Windows Command Prompt or PowerShell, run:

```bat
build_headless.bat
```

编译前请确认 TASKING 工具链和工程所需环境已经安装并配置。成功时应看到构建完成且没有 error；warning 也应尽量处理。

Make sure the TASKING toolchain and the required project environment are installed and configured. A successful build should finish without errors; warnings should also be investigated whenever possible.

## 烧录 | Flash

确认开发板已经连接，并检查电源、USB/调试器和电机驱动器接线，然后执行：

Connect the board and check the power, debugger, and motor-driver wiring before running:

```bat
flash.bat
```

烧录完成后，应确认编程、校验和 Flash 匹配均成功。第一次上电测试时，建议让驱动轮离地。

After flashing, verify that programming, verification, and Flash matching all succeed. Keep the drive wheels off the ground during the first power-on test.

## 基本测试 | Basic Tests

### 1. 上电安全检查 | Power-on Safety Check

确认上电后电机不应自行转动，PWM 初始输出应为零或处于安全状态。

The motors must remain stopped after power-on, and the initial PWM output must be zero or otherwise safe.

### 2. 电机开环检查 | Open-loop Motor Check

依次进行单轮、双轮、正转、反转和停止测试：

Test one wheel, both wheels, forward, reverse, and stop in sequence:

```text
只测试左轮
只测试右轮
两轮同时正转
两轮同时反转
发送停止命令并观察是否保持停止
```

正常现象是轮子对应关系正确、方向符合命令，停止后不会自动重新启动。

The expected result is correct wheel selection and direction, with no automatic restart after a stop command.

### 3. 编码器检查 | Encoder Check

低速转动轮子，检查编码器计数是否变化，并确认电机正方向对应正确的反馈符号。

Rotate the wheels slowly and verify that the encoder counts change and that the feedback sign matches the motor direction convention.

### 4. PID 速度检查 | PID Speed Check

逐步增加目标速度，观察实际速度是否跟随目标变化。正常现象包括：

Increase the target speed gradually and observe whether the measured speed follows it. Expected behavior includes:

```text
启动平稳
目标速度提高后实际转速提高
稳态时不会持续加速
左右轮方向一致
目标清零后电机停止
```

The controller should start smoothly, track speed changes, avoid continuous acceleration at steady state, keep both wheel directions consistent, and stop when the target is cleared.

### 5. 摄像头图像与 Otsu 阈值试跑 | Camera Image and Otsu Trial

当前固件用于调试摄像头，不驱动电机。屏幕改为横屏（160×128），摄像头 188×120 灰度图缩放到 160×102 全屏显示，不叠加任何文字。

The current firmware is for camera debugging and does not drive the motors. The display is switched to landscape (160×128), and the 188x120 gray frame is scaled to 160x102 with no text overlay.

```text
KEY4  灰度图 / 二值图 切换
KEY2  立即停止两轮（安全兜底，平时不会有动作）
```

二值化直接复用 Otsu 阈值：库函数 `tft180_show_gray_image()` 的最后参数为 0 输出灰阶，非 0 按该阈值输出黑白。

The binary view reuses the Otsu threshold: `tft180_show_gray_image()` draws grayscale when its last argument is 0 and binarizes with that value otherwise.

串口 115200 8N1，上电输出 `BOOT` 和 `CAM OK` / `CAM ERR`，可发送 `PING`、`STATUS`。

UART0 runs at 115200 8N1 and prints `BOOT` plus `CAM OK` / `CAM ERR`. `PING` and `STATUS` are accepted.

预期现象：

| 场景 | 预期 |
|---|---|
| 正常出图 | 灰度图有实时噪点，二值图黑白分明、边线连续 |
| 遮挡镜头 | 二值图接近全黑（阈值趋近 0） |
| 强光/白纸 | 二值图接近全白（阈值偏高） |
| 图像横向撕裂 | 整屏刷新耗时约 20 ms，期间新帧写入缓冲区，调试可接受 |
| 一直 `CAM ERR` | 摄像头未接、供电不足或排线松动 |

Expected behavior:

| Scene | Expectation |
|---|---|
| Normal capture | Live noise on the gray view, clean black/white edges on the binary view |
| Lens covered | Binary view nearly all black (threshold near 0) |
| Bright light or white paper | Binary view nearly all white (threshold high) |
| Horizontal tearing | Full-screen refresh takes about 20 ms while a new frame is written; acceptable for debugging |
| Persistent `CAM ERR` | Camera not connected, insufficient power, or a loose cable |

已知约束：

1. 屏幕方向必须在 `tft180_init()` 之前用 `tft180_set_dir()` 设定，否则 0x36 (MADCTL) 寄存器仍是旧方向。
2. 画面完全均匀时 Otsu 返回 0，因为没有任何阈值能同时得到两个非空类。
3. 单一全局阈值无法处理透视造成的不均匀光照，后续需要分区阈值或限定 ROI。

Known constraints:

1. The direction must be set with `tft180_set_dir()` before `tft180_init()`, otherwise the 0x36 (MADCTL) register keeps the old orientation.
2. A perfectly uniform frame yields 0, because no threshold produces two non-empty classes.
3. A single global threshold cannot handle illumination gradients caused by perspective; region-wise thresholds or a restricted ROI are needed later.

## 软件架构 | Software Architecture

项目采用分层和接口化设计：

The project uses a layered design with explicit interfaces:

```text
应用层 app.c
    ↓ 运动命令和运行模式
控制层 speed.c / pid.c
    ↓ 控制输出
设备层 motor.c / encoder / key.c
    ↓
TC264 外设和硬件
```

`app.c` 负责应用状态和运动命令；`speed.c` 负责读取反馈并运行左右轮速度控制；`pid.c` 提供通用 PID 算法；`motor.c` 负责将控制量转换为驱动器输出；中断统一放在 `user/isr.c`。

`chassis.c` 提供线速度/角速度、左右轮目标速度和转向角接口，当前使用抽象定点速度单位；`servo.c` 提供 50 Hz PWM 和渐变输出。底盘测试应用通过底盘接口统一控制驱动轮和舵机。

底盘模块同时管理整车转向限幅：转向角范围为 40° 到 140°，中位为 90°；`servo.c` 只负责通用舵机 PWM 和渐变输出。应用层应通过 `chassis_set_steering_angle()` 设置转向角，不应直接绕过底盘限幅调用舵机接口。

`app.c` manages application states and motion commands. `speed.c` reads feedback and runs the two wheel speed controllers. `pid.c` provides the reusable PID algorithm. `motor.c` converts control output into driver signals, while interrupt handlers are kept in `user/isr.c`.

`chassis.c` provides linear/angular velocity, wheel-speed, and steering-angle commands using abstract fixed-point units. `servo.c` provides 50 Hz PWM and ramp output. The chassis test application uses the chassis interface for both drive wheels and steering.

The chassis module also owns the vehicle steering limits: 40° to 140° with 90° as neutral. `servo.c` only provides generic servo PWM and ramp output. Application code should use `chassis_set_steering_angle()` so the chassis limit cannot be bypassed.
当前 PID 参数为：

The current PID parameters are:

```text
KP = 1000
KI = 40
KD = 0
输出范围 = -5000 到 5000
控制周期 = 10 ms
```

这些参数只适用于当前硬件和测试条件。更换电机、负载、供电或编码器后，需要重新验证和整定。

These values are specific to the current hardware and test conditions. Retune and verify the controller after changing the motors, load, power supply, or encoders.

## 协作与提交 | Collaboration and Commits

建议每个功能先在独立分支完成，再合并到 `main`。一次提交尽量只解决一个问题，并在提交说明中写清楚修改内容和验证方式。

Use a separate branch for each feature and merge it into `main` after review. Keep each commit focused on one change and describe both the modification and the verification method.

常用命令：

Common commands:

```bash
git status
git switch -c feature/your-feature
git diff
git add code user build_headless.bat flash.bat
git commit -m "feat: describe the change"
git push -u origin feature/your-feature
```

不要提交编译生成物、临时日志或个人工具配置。提交前检查 `git diff`，确认没有把无关文件加入暂存区。

Do not commit build artifacts, temporary logs, or personal tool configuration. Review `git diff` before committing and make sure unrelated files are not staged.
