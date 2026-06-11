# class1

第一次作业

ex01-C语言 HelloWorld 实验项目。

1.目录结构
- `ex01/hello.c` - HelloWorld 程序（支持用户输入）

2.版本记录
- v1.0: 基础 HelloWorld 输出
- v2.0: 增加键盘输入交互功能

3. 编译运行
```bash
  cd ex01
  gcc hello.c -o hello
  hello

第二次作业

ex02-esp32测试。

1.目录结构
- `ex02/sketch_jun11a.ino` - 测试程序程序（循环亮灯）

2.记录
- 改变"define LED_PIN"值，实现不同速度亮灯循环

3. 思考
- ### 问题1 波特率不一致的现象与原因
- 现象：串口监视器显示乱码。
- 原因：波特率是串口通信双方约定的传输速率，不一致时，数据采样节奏错位，无法正确解析字符。

- ### 问题2 修改 delay (1000) 数值对 LED 闪烁频率的影响
- 增大`delay()`数值 → LED闪烁变慢；
- 减小`delay()`数值 → LED闪烁变快。
