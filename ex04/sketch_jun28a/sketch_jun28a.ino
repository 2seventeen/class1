// ============================================
// 实验：基于触摸传感器的"自锁"开关
// ============================================

// 引脚定义
#define TOUCH_PIN 4    // 触摸引脚 T0 对应 GPIO4
#define LED_PIN 2      // 板载LED引脚 GPIO2

// 触摸阈值
#define THRESHOLD 300

// 软件防抖参数
#define DEBOUNCE_DELAY 200  // 防抖延时 200ms

// 状态变量
bool ledState = false;        // LED当前状态（false=灭, true=亮）
bool lastTouchState = false;  // 上一次触摸状态
bool currentTouchState = false; // 当前触摸状态

unsigned long lastDebounceTime = 0; // 上次触发时间

void setup() {
  // 初始化串口
  Serial.begin(115200);
  delay(1000);
  Serial.println("=== 触摸自锁开关实验 ===");
  Serial.println("请触摸 GPIO4 引脚，观察LED状态变化");

  // 设置LED引脚为输出
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // 初始状态：LED熄灭
}

void loop() {
  // 1. 读取触摸值
  int touchValue = touchRead(TOUCH_PIN);
  Serial.print("Touch Value: ");
  Serial.println(touchValue);

  // 2. 判断当前是否被触摸（触摸时值变小，小于阈值）
  if (touchValue < THRESHOLD) {
    currentTouchState = true;   // 被触摸
  } else {
    currentTouchState = false;  // 未触摸
  }

  // 3. 边缘检测 + 软件防抖
  // 检测"按下瞬间"：上一次未触摸，当前被触摸
  if (currentTouchState == true && lastTouchState == false) {
    
    // 检查防抖时间
    unsigned long currentTime = millis();
    if (currentTime - lastDebounceTime > DEBOUNCE_DELAY) {
      
      // 翻转LED状态
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState);
      
      // 更新防抖时间
      lastDebounceTime = currentTime;
      
      // 串口输出状态
      Serial.print(">>> 状态翻转！LED: ");
      Serial.println(ledState ? "ON" : "OFF");
    }
  }

  // 4. 更新上一次触摸状态
  lastTouchState = currentTouchState;

  // 采样间隔
  delay(50);
}