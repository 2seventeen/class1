#include <WiFi.h>
#include <WebServer.h>

// ==================== 配置区 ====================
// WiFi 设置
const char* ssid = "unknown";
const char* password = "9230423lm";

// 引脚定义
const int LED_PIN = 2;       // 板载LED引脚（GPIO2）
const int TOUCH_PIN = 4;     // 触摸引脚 T0 对应 GPIO4

// 触摸阈值（需根据实际测试调整，触摸时值会明显变小）
const int THRESHOLD = 300;    

// 报警闪烁间隔（毫秒），值越小闪烁越快
const int ALARM_BLINK_INTERVAL = 100; // 100ms = 高频闪烁

// ==================== 全局状态变量 ====================
// 系统状态枚举
enum SystemState {
  STATE_DISARMED,   // 撤防状态
  STATE_ARMED,      // 布防状态（等待触发）
  STATE_ALARM       // 报警状态（已触发，LED狂闪）
};

SystemState systemState = STATE_DISARMED;  // 初始状态：撤防

// 非阻塞闪烁相关变量
unsigned long lastBlinkTime = 0;
bool ledBlinkState = false;

WebServer server(80);

// ==================== 网页生成函数 ====================
String makePage() {
  String stateText;
  String stateColor;
  String btnArmStyle, btnDisarmStyle;

  switch (systemState) {
    case STATE_DISARMED:
      stateText = "撤防中 (Disarmed)";
      stateColor = "#28a745";  // 绿色
      btnArmStyle = "background:#007bff;color:#fff;";
      btnDisarmStyle = "background:#ccc;color:#888;cursor:not-allowed;";
      break;
    case STATE_ARMED:
      stateText = "布防中 (Armed)";
      stateColor = "#ffc107";  // 黄色
      btnArmStyle = "background:#ccc;color:#888;cursor:not-allowed;";
      btnDisarmStyle = "background:#dc3545;color:#fff;";
      break;
    case STATE_ALARM:
      stateText = "🚨 报警中 (ALARM!)";
      stateColor = "#dc3545";  // 红色
      btnArmStyle = "background:#ccc;color:#888;cursor:not-allowed;";
      btnDisarmStyle = "background:#dc3545;color:#fff;animation:pulse 0.5s infinite;";
      break;
  }

  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 安防报警器</title>
  <style>
    body { font-family: Arial, sans-serif; text-align: center; margin-top: 50px; background: #f5f5f5; }
    .container { max-width: 400px; margin: 0 auto; padding: 30px; background: #fff; border-radius: 15px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); }
    h1 { color: #333; margin-bottom: 10px; }
    .status-box { padding: 20px; border-radius: 10px; margin: 20px 0; font-size: 20px; font-weight: bold; color: #fff; background: )rawliteral" + stateColor + R"rawliteral(; transition: all 0.3s; }
    .btn { display: inline-block; padding: 15px 40px; margin: 10px; border: none; border-radius: 8px; font-size: 18px; cursor: pointer; text-decoration: none; transition: all 0.2s; }
    .btn:hover { transform: translateY(-2px); box-shadow: 0 4px 10px rgba(0,0,0,0.2); }
    .btn:active { transform: translateY(0); }
    .info { margin-top: 20px; color: #666; font-size: 14px; line-height: 1.6; }
    @keyframes pulse { 0% { transform: scale(1); } 50% { transform: scale(1.05); } 100% { transform: scale(1); } }
  </style>
</head>
<body>
  <div class="container">
    <h1>🔒 物联网安防报警器</h1>
    <div class="status-box">当前状态：)rawliteral" + stateText + R"rawliteral(</div>
    <a href="/arm"><button class="btn" style=")rawliteral" + btnArmStyle + R"rawliteral(" )rawliteral" + (systemState == STATE_DISARMED ? "" : "disabled") + R"rawliteral(>🔒 布防 (Arm)</button></a>
    <a href="/disarm"><button class="btn" style=")rawliteral" + btnDisarmStyle + R"rawliteral(" )rawliteral" + (systemState == STATE_DISARMED ? "disabled" : "") + R"rawliteral(>🔓 撤防 (Disarm)</button></a>
    <div class="info">
      <p>📡 ESP32 安防主机</p>
      <p>• 未布防时触摸引脚无反应</p>
      <p>• 布防后触碰引脚即触发报警</p>
      <p>• 报警后LED高频闪烁锁定</p>
      <p>• 只有点击"撤防"才能解除</p>
    </div>
  </div>
</body>
</html>
)rawliteral";
  return html;
}

// ==================== 路由处理函数 ====================
void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", makePage());
}

// 布防操作
void handleArm() {
  if (systemState == STATE_DISARMED) {
    systemState = STATE_ARMED;
    digitalWrite(LED_PIN, LOW);  // 布防时LED熄灭
    Serial.println("[系统] 已布防 - 等待触发");
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// 撤防操作
void handleDisarm() {
  if (systemState != STATE_DISARMED) {
    systemState = STATE_DISARMED;
    digitalWrite(LED_PIN, LOW);  // 撤防时LED熄灭
    ledBlinkState = false;
    Serial.println("[系统] 已撤防 - 系统重置");
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// ==================== 报警闪烁处理（非阻塞） ====================
void handleAlarmBlink() {
  if (systemState == STATE_ALARM) {
    unsigned long currentTime = millis();
    if (currentTime - lastBlinkTime >= ALARM_BLINK_INTERVAL) {
      lastBlinkTime = currentTime;
      ledBlinkState = !ledBlinkState;
      digitalWrite(LED_PIN, ledBlinkState ? HIGH : LOW);
    }
  }
}

// ==================== 触摸检测 ====================
void checkTouch() {
  // 只有在布防状态下才检测触摸
  if (systemState != STATE_ARMED) {
    return;
  }

  int touchValue = touchRead(TOUCH_PIN);
  // 调试输出（可选，调通后可注释掉）
  // Serial.print("Touch Value: ");
  // Serial.println(touchValue);

  // 触摸触发：读数小于阈值
  if (touchValue < THRESHOLD) {
    systemState = STATE_ALARM;
    lastBlinkTime = millis();  // 重置闪烁计时器
    ledBlinkState = true;
    digitalWrite(LED_PIN, HIGH);
    Serial.println("[警报] 触摸触发！进入报警状态！");
  }
}

// ==================== 初始化 ====================
void setup() {
  Serial.begin(115200);
  delay(1000);

  // 初始化LED引脚
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 连接WiFi
  WiFi.begin(ssid, password);
  Serial.print("连接WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi连接成功");
  Serial.print("访问地址: http://");
  Serial.println(WiFi.localIP());

  // 注册路由
  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.begin();

  Serial.println("安防系统初始化完成 - 当前状态：撤防");
  Serial.println("请在浏览器中访问上述IP地址进行操作");
}

// ==================== 主循环 ====================
void loop() {
  // 1. 处理Web客户端请求
  server.handleClient();

  // 2. 检测触摸（仅在布防状态下有效）
  checkTouch();

  // 3. 处理报警闪烁（非阻塞，使用millis）
  handleAlarmBlink();

  // 小延时避免看门狗复位，同时保证响应性
  delay(10);
}