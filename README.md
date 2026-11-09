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
    speed_control.c/.h       速度控制 / Speed control
    chassis.c/.h             底盘接口 / Chassis interface
    servo.c/.h               舵机接口 / Servo interface
    pid.c/.h                 PID 算法 / PID algorithm
    key_module.c/.h          按键模块 / Key module
    motion_interface.h       运动接口 / Motion interface

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

## 软件架构 | Software Architecture

项目采用分层和接口化设计：

The project uses a layered design with explicit interfaces:

```text
应用层 app.c
    ↓ 运动命令和运行模式
控制层 speed_control.c / pid.c
    ↓ 控制输出
设备层 motor.c / encoder / key_module.c
    ↓
TC264 外设和硬件
```

`app.c` 负责应用状态和运动命令；`speed_control.c` 负责读取反馈并运行左右轮速度控制；`pid.c` 提供通用 PID 算法；`motor.c` 负责将控制量转换为驱动器输出；中断统一放在 `user/isr.c`。

`chassis.c` 提供线速度/角速度和左右轮目标速度接口，当前使用抽象定点速度单位；`servo.c` 提供角度限幅、回中和 50 Hz PWM 输出。两者目前只加入模块接口，尚未接入应用层控制流程。

`app.c` manages application states and motion commands. `speed_control.c` reads feedback and runs the two wheel speed controllers. `pid.c` provides the reusable PID algorithm. `motor.c` converts control output into driver signals, while interrupt handlers are kept in `user/isr.c`.

`chassis.c` provides linear/angular velocity and wheel-speed commands using abstract fixed-point units. `servo.c` provides angle limiting, centering, and 50 Hz PWM output. These modules currently exist as interfaces and are not connected to the application control flow.

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

### 5. 舵机按键测试（驱动轮关闭） | Servo Key Test (Drive Disabled)

当前测试固件已关闭驱动轮，只保留舵机测试。烧录后不需要 UART。首次测试时舵机不要连接机械负载：

The test firmware disables the drive wheels and keeps only servo testing enabled. UART is not required. Remove the servo load during the first test:

```text
KEY4 短按：90° -> 40° -> 140° -> 90°
KEY4 长按：舵机回中，并立即停止驱动轮
KEY2 短按或长按：停止转动并保持当前舵机状态

舵机每 50 ms 调整 1°，约为每秒 20°，不会快速撞向目标角度；屏幕显示 `CMD` 软件当前输出角度和 `TGT` 目标角度。当前对称限位为 40° 到 140°，KEY4 测试两侧限位。普通舵机没有角度反馈，屏幕数值不是机械实际角度。

The servo moves by 1° every 50 ms, approximately 20° per second, instead of jumping to the target. The display shows the current software output angle as `CMD` and the target as `TGT`. The symmetric limits are 40° and 140°, and KEY4 tests both sides. A standard servo has no angle feedback, so these values are not measured mechanical angles.
```

建议测试顺序：

1. 上电后确认驱动轮不转。
2. 短按 KEY4，观察屏幕角度逐步变化到 40°。
3. 再次短按 KEY4，观察屏幕角度逐步变化到 140°。
4. 再次短按 KEY4，观察屏幕角度逐步回到 90°。
5. 短按 KEY4，确认舵机回中到当前配置的中位角。
6. 舵机动作异常时立即断电。

Recommended order:

1. Confirm that the drive wheels remain stopped after power-on.
2. Press KEY4 once and observe the displayed angle gradually move to 40°.
3. Press KEY4 again and observe the displayed angle gradually move to 140°.
4. Press KEY4 again and observe the displayed angle gradually return to 90°.
5. Press KEY4 and confirm that the servo returns to the configured neutral angle.
6. Remove power immediately if the servo behaves unexpectedly.

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
