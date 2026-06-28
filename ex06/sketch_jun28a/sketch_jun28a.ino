// ========== 引脚定义 ==========
const int ledA_Pin = 4;   // LED A 连接到 GPIO4
const int ledB_Pin = 5;   // LED B 连接到 GPIO5

// ========== PWM 属性 ==========
const int freq = 5000;       // 频率 5000Hz
const int resolution = 8;    // 分辨率 8位 (0-255)

// ========== millis() 定时相关变量 ==========
const unsigned long stepInterval = 10;  // 每步间隔 10ms（控制渐变速度）
unsigned long previousMillis = 0;        // 记录上一次更新时间

int dutyCycle = 0;           // 当前占空比 (0-255)
bool increasing = true;      // true: 递增, false: 递减

void setup() {
  Serial.begin(115200);

  // 初始化两个引脚的 PWM
  ledcAttach(ledA_Pin, freq, resolution);
  ledcAttach(ledB_Pin, freq, resolution);

  Serial.println("Police dual-flash PWM (non-blocking) started");
}

void loop() {
  unsigned long currentMillis = millis();

  // 检查是否到达下一步更新时间
  if (currentMillis - previousMillis >= stepInterval) {
    previousMillis = currentMillis;  // 更新时间戳

    // 更新占空比
    if (increasing) {
      dutyCycle++;
      if (dutyCycle >= 255) {
        dutyCycle = 255;
        increasing = false;  // 到达峰值，开始递减
      }
    } else {
      dutyCycle--;
      if (dutyCycle <= 0) {
        dutyCycle = 0;
        increasing = true;   // 到达谷底，开始递增
      }
    }

    // 反相输出到两个 LED
    int dutyA = dutyCycle;       // A 的亮度
    int dutyB = 255 - dutyCycle; // B 的亮度（反相）

    ledcWrite(ledA_Pin, dutyA);
    ledcWrite(ledB_Pin, dutyB);
  }


}