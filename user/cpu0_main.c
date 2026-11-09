/*********************************************************************************************************************
* TC264 Opensourec Library 即（TC264 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 TC264 开源库的一部分
*
* TC264 开源库是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU 通用公共许可证）的条款
* 即 GPL 的第 3 版（GPL-3.0）或（您选择的）任何后续版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但不提供任何保证
* 甚至不包含适销性或特定用途适用性的隐含保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 副本
* 如果没有，请参阅 <https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL-3.0 开源许可证协议，以上许可声明为译文
* 英文许可声明位于 libraries/doc/GPL3_permission_statement.txt
* 许可证副本位于 libraries 目录
* 欢迎使用和传播本程序；修改时请保留逐飞科技版权声明
*
* 文件名称          cpu0_main
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc/version.txt
* 开发环境          ADS v1.8.0
* 适用平台          TC264D
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2022-09-15       pudding             first version
*********************************************************************************************************************/

#include "zf_common_headfile.h"
#include "app.h"

#pragma section all "cpu0_dsram"
// 从此处到“#pragma section all restore”之间定义的全局变量放入 CPU0 DSRAM。

// *************************** 调试操作说明 ***************************
// KEY3 短按按速度档循环：停止、750、1500、2250、5000，然后回到停止。
// KEY4 短按切换正转/反转，长按滑行停车。上电后默认停止。
// KEY3：P11.2；KEY4：P11.3；按下接地，由 1 ms PIT 中断扫描。
// 左电机：PWM P02.7、方向 P02.6；右电机：PWM P02.5、方向 P02.4。

// **************************** 主函数 ****************************
int core0_main(void)
{
    clock_init();                   // 初始化系统时钟
    debug_init();                   // 初始化默认调试串口

    app_init();                     // 初始化按键、电机和 1 ms 周期定时器
    cpu_wait_event_ready();         // 等待其他 CPU 核心完成初始化

    while(TRUE)
    {
        app_task();                 // 在主循环处理按键事件并更新电机输出
    }
}

#pragma section all restore
