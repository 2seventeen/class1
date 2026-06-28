// LED引脚 ESP32板载LED默认GPIO2
const uint8_t ledPin = 2;

// 时间参数定义
const unsigned short T_SHORT = 200;    // 短亮时长
const unsigned short T_LONG = 600;     // 长亮时长
const unsigned short T_GAP = 200;      // 单次闪烁间间隔
const unsigned short T_GROUP_GAP = 600; // 组与组间隔（3短完到3长、3长完到3短）
const unsigned short T_LOOP_GAP = 2000;// 一轮SOS全部完成后的长停顿

// 状态机变量
unsigned long lastTime = 0;
bool ledOn = false;
// 阶段：0=三短S 1=三长O 2=三短S 3=等待一轮结束
uint8_t stage = 0;
uint8_t flashCnt = 0; // 当前阶段闪烁计数
unsigned long duration = T_SHORT; // 当前亮/灭持续时间

void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void loop() {
  unsigned long now = millis();
  if (now - lastTime < duration) return; // 时间未到，直接退出

  lastTime = now;
  ledOn = !ledOn;
  digitalWrite(ledPin, ledOn);

  if (ledOn) {
    // 开灯：设置本次亮灯时长
    if (stage == 0 || stage == 2) {
      duration = T_SHORT; // S短闪
    } else {
      duration = T_LONG;  // O长闪
    }
  } else {
    // 关灯，判断下一阶段
    flashCnt++;
    if (flashCnt >= 3) {
      // 当前一组3次闪烁完成
      flashCnt = 0;
      // 切换阶段，设置组间隔
      duration = T_GROUP_GAP;
      stage++;
      if (stage > 2) {
        // 完整一轮SOS结束，进入长等待
        stage = 3;
        duration = T_LOOP_GAP;
      }
    } else {
      // 同组内两次闪烁之间的短间隔
      duration = T_GAP;
    }
    // 等待长间隔结束，重置回到第一组三短闪
    if (stage == 3 && duration == T_LOOP_GAP) {
      stage = 0;
      flashCnt = 0;
    }
  }
}