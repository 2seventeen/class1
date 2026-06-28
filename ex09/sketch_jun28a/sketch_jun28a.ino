#include <WiFi.h>
#include <WebServer.h>

// ==================== 配置区 ====================
const char* ssid = "unknown";      // 修改为你的WiFi名称
const char* password = "9230423lm";  // 修改为你的WiFi密码

// 引脚定义
const int LED_PIN = 2;      // 板载LED，GPIO2
const int TOUCH_PIN = 4;    // 触摸引脚T0对应GPIO4

// 触摸阈值（已改为300）
int threshold = 300;

// Web服务器
WebServer server(80);

// ==================== 页面生成函数 ====================
String makePage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 实时传感器仪表盘</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    body {
      font-family: 'Microsoft YaHei', Arial, sans-serif;
      background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
      color: #eee;
      min-height: 100vh;
      display: flex;
      flex-direction: column;
      align-items: center;
      justify-content: center;
    }
    .container {
      text-align: center;
      padding: 40px;
    }
    h1 {
      font-size: 28px;
      margin-bottom: 10px;
      color: #00d4ff;
      text-shadow: 0 0 10px rgba(0, 212, 255, 0.3);
    }
    .subtitle {
      color: #888;
      font-size: 14px;
      margin-bottom: 40px;
    }
    .dashboard {
      background: rgba(255, 255, 255, 0.05);
      border-radius: 20px;
      padding: 40px;
      border: 1px solid rgba(255, 255, 255, 0.1);
      box-shadow: 0 8px 32px rgba(0, 0, 0, 0.3);
      min-width: 320px;
    }
    .value-label {
      font-size: 16px;
      color: #aaa;
      margin-bottom: 10px;
    }
    .value-display {
      font-size: 72px;
      font-weight: bold;
      color: #00ff88;
      font-family: 'Courier New', monospace;
      transition: color 0.3s ease;
      text-shadow: 0 0 20px rgba(0, 255, 136, 0.4);
    }
    .unit {
      font-size: 18px;
      color: #666;
      margin-top: 5px;
    }
    .status-bar {
      margin-top: 30px;
      display: flex;
      justify-content: center;
      gap: 20px;
      flex-wrap: wrap;
    }
    .status-item {
      background: rgba(255, 255, 255, 0.08);
      padding: 15px 25px;
      border-radius: 10px;
      min-width: 120px;
    }
    .status-title {
      font-size: 12px;
      color: #888;
      margin-bottom: 5px;
    }
    .status-value {
      font-size: 18px;
      font-weight: bold;
    }
    .led-on {
      color: #ffcc00;
      text-shadow: 0 0 10px rgba(255, 204, 0, 0.5);
    }
    .led-off {
      color: #555;
    }
    .update-time {
      margin-top: 20px;
      font-size: 12px;
      color: #555;
    }
    .progress-bar {
      width: 100%;
      height: 8px;
      background: rgba(255, 255, 255, 0.1);
      border-radius: 4px;
      margin-top: 20px;
      overflow: hidden;
    }
    .progress-fill {
      height: 100%;
      background: linear-gradient(90deg, #00ff88, #00d4ff);
      border-radius: 4px;
      transition: width 0.3s ease;
    }
    .threshold-info {
      margin-top: 15px;
      font-size: 12px;
      color: #666;
    }
    .error {
      color: #ff6b6b;
      font-size: 14px;
      margin-top: 20px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>ESP32 实时传感器仪表盘</h1>
    <p class="subtitle">触摸传感器数据实时监控</p>
    
    <div class="dashboard">
      <div class="value-label">当前触摸数值</div>
      <div class="value-display" id="touchValue">--</div>
      <div class="unit">(数值越小表示越靠近)</div>
      
      <div class="progress-bar">
        <div class="progress-fill" id="progressBar" style="width: 0%"></div>
      </div>
      <div class="threshold-info">阈值: 300 | 范围: 0 ~ 100</div>
      
      <div class="status-bar">
        <div class="status-item">
          <div class="status-title">LED状态</div>
          <div class="status-value" id="ledStatus">--</div>
        </div>
        <div class="status-item">
          <div class="status-title">触摸状态</div>
          <div class="status-value" id="touchStatus">--</div>
        </div>
      </div>
      
      <div class="update-time" id="updateTime">等待数据...</div>
      <div class="error" id="errorMsg"></div>
    </div>
  </div>

  <script>
    const THRESHOLD = 300;
    const MAX_VALUE = 100;
    
    function updateData() {
      fetch('/api/touch')
        .then(response => {
          if (!response.ok) {
            throw new Error('网络响应异常: ' + response.status);
          }
          return response.json();
        })
        .then(data => {
          const valueEl = document.getElementById('touchValue');
          const ledEl = document.getElementById('ledStatus');
          const touchEl = document.getElementById('touchStatus');
          const timeEl = document.getElementById('updateTime');
          const errorEl = document.getElementById('errorMsg');
          const progressEl = document.getElementById('progressBar');
          
          // 更新数值显示
          valueEl.textContent = data.touchValue;
          
          // 根据数值变化颜色
          if (data.touchValue < THRESHOLD) {
            valueEl.style.color = '#ff6b6b';
          } else {
            valueEl.style.color = '#00ff88';
          }
          
          // 更新LED状态
          if (data.ledState) {
            ledEl.textContent = '点亮';
            ledEl.className = 'status-value led-on';
          } else {
            ledEl.textContent = '熄灭';
            ledEl.className = 'status-value led-off';
          }
          
          // 更新触摸状态
          if (data.isTouching) {
            touchEl.textContent = '检测到触摸';
            touchEl.style.color = '#ff6b6b';
          } else {
            touchEl.textContent = '未触摸';
            touchEl.style.color = '#00ff88';
          }
          
          // 更新进度条
          let percent = ((MAX_VALUE - data.touchValue) / MAX_VALUE) * 100;
          if (percent < 0) percent = 0;
          if (percent > 100) percent = 100;
          progressEl.style.width = percent + '%';
          
          // 更新时间
          const now = new Date();
          timeEl.textContent = '最后更新: ' + now.toLocaleTimeString();
          
          // 清除错误信息
          errorEl.textContent = '';
        })
        .catch(err => {
          document.getElementById('errorMsg').textContent = 
            '数据获取失败: ' + err.message;
        });
    }
    
    // 页面加载后立即获取一次
    updateData();
    
    // 每200毫秒轮询一次，实现实时刷新
    setInterval(updateData, 200);
  </script>
</body>
</html>
)rawliteral";
  return html;
}

// ==================== 路由处理函数 ====================

// 根路径：返回主页面
void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", makePage());
}

// API接口：返回JSON格式的传感器数据
void handleTouchData() {
  int touchValue = touchRead(TOUCH_PIN);
  bool isTouching = (touchValue < threshold);
  bool ledState = digitalRead(LED_PIN);
  
  // 根据触摸值自动控制LED
  if (isTouching) {
    digitalWrite(LED_PIN, HIGH);
    ledState = true;
  } else {
    digitalWrite(LED_PIN, LOW);
    ledState = false;
  }
  
  // 构建JSON响应
  String json = "{";
  json += "\"touchValue\":" + String(touchValue) + ",";
  json += "\"isTouching\":" + String(isTouching ? "true" : "false") + ",";
  json += "\"ledState\":" + String(ledState ? "true" : "false") + ",";
  json += "\"threshold\":" + String(threshold);
  json += "}";
  
  // 添加CORS头，允许跨域访问
  server.sendHeader("Access-Control-Allow-Origin", "*");
  server.sendHeader("Content-Type", "application/json");
  server.send(200, "application/json", json);
}

// 处理404
void handleNotFound() {
  server.send(404, "text/plain", "Not Found");
}

// ==================== 初始化与主循环 ====================
void setup() {
  Serial.begin(115200);
  delay(1000);
  
  // 初始化LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  Serial.println("\n========== ESP32 实时传感器仪表盘 ==========");
  Serial.println("正在连接WiFi...");
  
  // 连接WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi连接成功!");
  Serial.print("IP地址: http://");
  Serial.println(WiFi.localIP());
  Serial.print("触摸引脚: GPIO");
  Serial.println(TOUCH_PIN);
  Serial.print("LED引脚: GPIO");
  Serial.println(LED_PIN);
  Serial.println("==========================================");
  
  // 设置路由
  server.on("/", handleRoot);
  server.on("/api/touch", handleTouchData);
  server.onNotFound(handleNotFound);
  
  // 启动服务器
  server.begin();
  Serial.println("Web服务器已启动");
}

void loop() {
  // 处理客户端请求
  server.handleClient();
  
  // 可选：在串口也输出调试信息
  static unsigned long lastPrint = 0;
  if (millis() - lastPrint > 1000) {
    lastPrint = millis();
    int touchValue = touchRead(TOUCH_PIN);
    Serial.print("Touch Value: ");
    Serial.print(touchValue);
    Serial.print(" | LED: ");
    Serial.println(digitalRead(LED_PIN) ? "ON" : "OFF");
  }
}