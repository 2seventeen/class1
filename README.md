# class1

第一次作业

ex00-C语言 HelloWorld 实验项目。

1.目录结构
- `ex00/hello.c` - HelloWorld 程序（支持用户输入）

2.版本记录
- v1.0: 基础 HelloWorld 输出
- v2.0: 增加键盘输入交互功能

3. 编译运行
```bash
  cd ex01
  gcc hello.c -o hello
  hello

第二次作业

ex01-esp32测试。

1.目录结构
- `ex02/sketch_jun11a.ino` - 测试程序程序（循环亮灯）


2.记录
- 改变"define LED_PIN"值，实现不同速度亮灯循环

3. 思考

- 问题1 波特率不一致的现象与原因
- 现象：串口监视器显示乱码。
- 原因：波特率是串口通信双方约定的传输速率，不一致时，数据采样节奏错位，无法正确解析字符。

- 问题2 修改 delay (1000) 数值对 LED 闪烁频率的影响
- 增大`delay()`数值 → LED闪烁变慢；
- 减小`delay()`数值 → LED闪烁变快。

第三次作业

ex02-esp32-实现控制LED以1Hz的频率稳定闪烁。

1.目录结构
- `ex02/sketch_jun19b-1Hzflash.ino` - 实现固定频率闪烁。
- `1Hz闪亮.mp4`-结果展示

ex03-esp32-实现控制LED产生SOS闪烁信息。

1.目录结构
- `ex03/sketch_jun19c-sos.ino` - 实现彩灯sos闪烁。
- `sos.mp4`-结果展示

lab02-esp32-LED闪烁控制。

1.目录结构
- `lab02/sketch_jun19a.ino` - 实现最基本的 LED 点亮和熄灭控制。
- `


2.记录
- 改变"delay()"值，实现不同速度亮灯循环

lab03-esp32-实现呼吸灯效果。

1.目录结构
- `lab03/sketch_jun18c.ino` - 实现彩灯呼吸。
- `lab03/sketch_jun18a.ino` - 实现两个彩灯同时相反呼吸。
- `lab03/sketch_jun18b.ino` - 实现触控彩灯亮灭。


2.记录
- 改变"delay()"值，实现不同速度亮灯循环
-注意"threshold"阈值取值


