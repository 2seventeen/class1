// 定义LED引脚
const uint8_t ledPin = 2;
// 记录上一次状态切换的时间
unsigned long lastToggleTime = 0;
// 闪烁间隔 500ms（1Hz，1秒一个周期，半秒切换一次）
const unsigned long interval = 500;
// LED状态标记
bool ledState = LOW;

void setup() {
  pinMode(ledPin, OUTPUT);
}

void loop() {
  // 获取当前系统毫秒时间
  unsigned long currentTime = millis();
  
  // 判断是否到达切换时间
  if (currentTime - lastToggleTime >= interval) {
    // 更新上次切换时间戳
    lastToggleTime = currentTime;
    // 翻转LED状态
    ledState = !ledState;
    digitalWrite(ledPin, ledState);
  }
}