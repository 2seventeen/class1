// 定义触摸引脚 (T0对应GPIO4)
#define TOUCH_PIN 4
// 定义LED引脚 (ESP32 DevKit板载LED通常是GPIO2)
#define LED_PIN 2

// 阈值，需要通过串口监视器观察并调整
int threshold = 20; 

// LED状态标志
volatile bool ledState = false;

// 防抖变量
volatile unsigned long lastTouchTime = 0;
const unsigned long debounceDelay = 300; // 防抖时间300ms

// 触摸中断服务程序
void IRAM_ATTR onTouch() {
  unsigned long currentTime = millis();
  
  // 简单的软件防抖
  if (currentTime - lastTouchTime > debounceDelay) {
    ledState = !ledState; // 翻转LED状态
    lastTouchTime = currentTime;
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000); // 等待串口稳定

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // 初始状态LED灭

  // 读取初始触摸值，用于设置阈值参考
  int baseValue = touchRead(TOUCH_PIN);
  Serial.print("Base Touch Value: ");
  Serial.println(baseValue);
  
  // 设置触摸中断阈值（通常触摸时数值会下降）
  // 阈值建议设为基准值的70%左右，或根据实际观察调整
  threshold = baseValue * 0.7;
  Serial.print("Threshold set to: ");
  Serial.println(threshold);

  // 附加触摸中断：当触摸值小于阈值时触发中断
  touchAttachInterrupt(TOUCH_PIN, onTouch, threshold);

  Serial.println("Touch interrupt ready. Touch the pin to toggle LED.");
}

void loop() {
  // 在主循环中更新LED状态
  digitalWrite(LED_PIN, ledState ? HIGH : LOW);

  // 可选：打印当前触摸值用于调试
  int touchValue = touchRead(TOUCH_PIN);
  Serial.print("Touch Value: ");
  Serial.print(touchValue);
  Serial.print(" | LED: ");
  Serial.println(ledState ? "ON" : "OFF");
  
  delay(200); // 适当延时，避免串口输出过快
}