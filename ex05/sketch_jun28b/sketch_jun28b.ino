/*
 * 多档位触摸调速呼吸灯
 */

#define TOUCH_PIN 4
#define LED_PIN 2
#define THRESHOLD 500

// 速度档位：1=慢, 2=中, 3=快
int speedLevel = 1;
const int MAX_LEVEL = 3;

// 各档位步进间隔（毫秒）：数值越大呼吸越慢
const int stepIntervals[] = {0, 50, 15, 3};

// PWM参数
const int freq = 5000;
const int resolution = 8;

// 触摸消抖
unsigned long lastTouchTime = 0;
const unsigned long debounceDelay = 300;
volatile bool touchDetected = false;

// ========== 呼吸灯状态变量 ==========
int dutyCycle = 0;              // 当前占空比
bool breathingUp = true;        // true=变亮, false=变暗
unsigned long lastStepTime = 0; // 上次步进时间

// 前向声明
void IRAM_ATTR onTouch();

void setup() {
  Serial.begin(115200);
  delay(1000);

  ledcAttach(LED_PIN, freq, resolution);
  touchAttachInterrupt(TOUCH_PIN, onTouch, THRESHOLD);

  Serial.println("=== 多档位触摸调速呼吸灯（非阻塞版）===");
  showStatus();
}

void IRAM_ATTR onTouch() {
  unsigned long currentTime = millis();
  if (currentTime - lastTouchTime > debounceDelay) {
    lastTouchTime = currentTime;
    touchDetected = true;
  }
}

void loop() {
  // ========== 1. 立即响应触摸事件（每次loop都检查）==========
  if (touchDetected) {
    touchDetected = false;
    
    // 切换档位
    speedLevel++;
    if (speedLevel > MAX_LEVEL) speedLevel = 1;
    
    Serial.println(">>> 触摸触发！档位切换 <<<");
    showStatus();
  }

  // ========== 2. 非阻塞式呼吸灯步进 ==========
  unsigned long currentTime = millis();
  
  // 检查是否到达步进时间
  if (currentTime - lastStepTime >= stepIntervals[speedLevel]) {
    lastStepTime = currentTime;

    // 写入当前占空比
    ledcWrite(LED_PIN, dutyCycle);

    // 更新占空比和方向
    if (breathingUp) {
      dutyCycle++;
      if (dutyCycle >= 255) {
        dutyCycle = 255;
        breathingUp = false;  // 到达最亮，开始变暗
      }
    } else {
      dutyCycle--;
      if (dutyCycle <= 0) {
        dutyCycle = 0;
        breathingUp = true;   // 到达最暗，开始变亮
        
        // 可选：输出完成一个周期
        static int cycleCount = 0;
        cycleCount++;
        Serial.print("呼吸周期 #");
        Serial.print(cycleCount);
        Serial.print(" 完成 | 档位: ");
        Serial.println(speedLevel);
      }
    }
  }

  // loop() 立即结束，回到开头检查触摸 → 响应极快！
}

// 显示当前状态
void showStatus() {
  Serial.print("当前档位: ");
  Serial.println(speedLevel);
  Serial.print("步进间隔: ");
  Serial.print(stepIntervals[speedLevel]);
  Serial.println(" ms");
  Serial.println("----------------------------");
}