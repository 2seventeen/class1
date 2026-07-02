#include <WiFi.h>
#include <WebServer.h>

// ==================== AP热点配置区 ====================
const char* ap_ssid = "ESP32-LAB17";    // 热点名称
const char* ap_password = "12345678";      // 热点密码（至少8位）

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
  String alertBlock = "";
  String jsAlertScript = "";

  switch (systemState) {
    case STATE_DISARMED:
      stateText = "撤防中 (Disarmed)";
      stateColor = "#28a745";  // 绿色
      btnArmStyle = "background:#007bff;color:#fff;";
      btnDisarmStyle = "background:#ccc;color:#888;cursor:not-allowed;";
      alertBlock = "";
      jsAlertScript = "";
      break;
    case STATE_ARMED:
      stateText = "布防中 (Armed)";
      stateColor = "#ffc107";  // 黄色
      btnArmStyle = "background:#ccc;color:#888;cursor:not-allowed;";
      btnDisarmStyle = "background:#dc3545;color:#fff;";
      alertBlock = "";
      jsAlertScript = "";
      break;
    case STATE_ALARM:
      stateText = "🚨 报警中 (ALARM!)";
      stateColor = "#dc3545";  // 红色
      btnArmStyle = "background:#ccc;color:#888;cursor:not-allowed;";
      btnDisarmStyle = "background:#dc3545;color:#fff;animation:pulse 0.5s infinite;";
      // 页面红色提示条
      alertBlock = R"rawliteral(
      <div class="invade-alert">
        ⚠️ 检测到入侵！安防警报已触发
      </div>
      )rawliteral";
      // JS弹窗脚本，仅提示检测到入侵，删除撤防相关文字
      jsAlertScript = R"rawliteral(
      <script>
        window.onload = function(){
          alert("【入侵警报】检测到非法闯入！");
        }
      </script>
      )rawliteral";
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
    .container { max-width: 420px; margin: 0 auto; padding: 30px; background: #fff; border-radius: 15px; box-shadow: 0 4px 15px rgba(0,0,0,0.1); }
    h1 { color: #333; margin-bottom: 10px; }
    .status-box { padding: 20px; border-radius: 10px; margin: 20px 0; font-size: 20px; font-weight: bold; color: #fff; background: )rawliteral" + stateColor + R"rawliteral(; transition: all 0.3s; }
    .btn { display: inline-block; padding: 15px 30px; margin: 10px; border: none; border-radius: 8px; font-size: 16px; cursor: pointer; text-decoration: none; transition: all 0.2s; }
    .btn:hover { transform: translateY(-2px); box-shadow: 0 4px 10px rgba(0,0,0,0.2); }
    .btn:active { transform: translateY(0); }
    .info { margin-top: 20px; color: #666; font-size: 14px; line-height: 1.6; }
    @keyframes pulse { 0% { transform: scale(1); } 50% { transform: scale(1.05); } 100% { transform: scale(1); } }
    /* 入侵警报闪烁提示条 */
    .invade-alert {
      background-color: #ff2222;
      color: #ffffff;
      font-size: 18px;
      font-weight: bold;
      padding: 16px;
      border-radius: 10px;
      margin: 20px 0;
      animation: alertFlash 0.8s infinite alternate;
    }
    @keyframes alertFlash {
      from { opacity: 1; }
      to { opacity: 0.6; }
    }
  </style>
</head>
)rawliteral" + jsAlertScript + R"rawliteral(
<body>
  <div class="container">
    <h1>🔒 物联网安防报警器</h1>
    <div class="status-box">当前状态：)rawliteral" + stateText + R"rawliteral(</div>
    )rawliteral" + alertBlock + R"rawliteral(
    <div>
      <a href="/arm"><button class="btn" style=")rawliteral" + btnArmStyle + R"rawliteral(" )rawliteral" + (systemState == STATE_DISARMED ? "" : "disabled") + R"rawliteral(>🔒 布防 (Arm)</button></a>
      <a href="/disarm"><button class="btn" style=")rawliteral" + btnDisarmStyle + R"rawliteral(" )rawliteral" + (systemState == STATE_DISARMED ? "disabled" : "") + R"rawliteral(>🔓 撤防 (Disarm)</button></a>
    </div>
    <div class="info">
      <p>📡 ESP32 AP热点模式</p>
      <p>• 手机连接热点：ESP32_Alarm_AP</p>
      <p>• 访问地址：192.168.4.1</p>
      <p>• 仅布防状态下触摸引脚会触发入侵警报</p>
      <p>• 报警页面自动弹出入侵警告弹窗+红色闪烁提示</p>
      <p>• 点击撤防即可解除警报</p>
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
    digitalWrite(LED_PIN, LOW);
    Serial.println("[系统] 已布防 - 等待触发");
  }
  server.sendHeader("Location", "/");
  server.send(303);
}

// 撤防操作
void handleDisarm() {
  if (systemState != STATE_DISARMED) {
    systemState = STATE_DISARMED;
    digitalWrite(LED_PIN, LOW);
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
  if (systemState != STATE_ARMED) {
    return;
  }

  int touchValue = touchRead(TOUCH_PIN);
  if (touchValue < THRESHOLD) {
    systemState = STATE_ALARM;
    lastBlinkTime = millis();
    ledBlinkState = true;
    digitalWrite(LED_PIN, HIGH);
    Serial.println("[警报] 触摸触发！进入报警状态！");
  }
}

// ==================== 初始化 ====================
void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // 创建AP热点
  Serial.print("正在创建AP热点: ");
  Serial.println(ap_ssid);
  WiFi.softAP(ap_ssid, ap_password, 1, 0, 4);
  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP访问地址: http://");
  Serial.println(apIP);

  // 注册网页路由（移除手动报警路由）
  server.on("/", handleRoot);
  server.on("/arm", handleArm);
  server.on("/disarm", handleDisarm);
  server.begin();

  Serial.println("安防系统初始化完成 - 当前状态：撤防");
}

// ==================== 主循环 ====================
void loop() {
  server.handleClient();
  checkTouch();
  handleAlarmBlink();
  delay(10);
}