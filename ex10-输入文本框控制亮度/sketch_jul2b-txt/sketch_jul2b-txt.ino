#include <WiFi.h>
#include <WebServer.h>

// ========== AP热点配置 ==========
const char* ap_ssid = "ESP32-LAB17";    // 热点名称
const char* ap_password = "12345678";    // 热点密码，至少8位

// ========== LED 配置 ==========
const int LED_PIN = 2;
const int PWM_FREQ = 5000;
const int PWM_RESOLUTION = 8;

// ========== 全局变量 ==========
WebServer server(80);

// ========== 生成网页 ==========
String makePage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>ESP32 无极调光器</title>
  <style>
    * { margin: 0; padding: 0; box-sizing: border-box; }
    body {
      font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, Arial, sans-serif;
      background: linear-gradient(135deg, #0f0c29 0%, #302b63 50%, #24243e 100%);
      min-height: 100vh;
      display: flex;
      justify-content: center;
      align-items: center;
      color: #fff;
      overflow: hidden;
    }
    .container {
      background: rgba(255,255,255,0.08);
      backdrop-filter: blur(20px);
      border-radius: 30px;
      padding: 50px 40px;
      box-shadow: 0 25px 50px rgba(0,0,0,0.4), inset 0 1px 0 rgba(255,255,255,0.1);
      text-align: center;
      max-width: 420px;
      width: 90%;
      border: 1px solid rgba(255,255,255,0.1);
    }
    h1 {
      font-size: 26px;
      font-weight: 600;
      background: linear-gradient(90deg, #00d4ff, #7b2ff7);
      -webkit-background-clip: text;
      -webkit-text-fill-color: transparent;
      margin-bottom: 8px;
      letter-spacing: 1px;
    }
    .subtitle {
      color: rgba(255,255,255,0.5);
      font-size: 13px;
      margin-bottom: 35px;
      letter-spacing: 0.5px;
    }
    .led-ring {
      width: 100px;
      height: 100px;
      border-radius: 50%;
      margin: 0 auto 30px;
      background: radial-gradient(circle, rgba(0,212,255,0.3) 0%, transparent 70%);
      border: 3px solid rgba(0,212,255,0.2);
      box-shadow: 0 0 40px rgba(0,212,255,0.1);
      transition: all 0.05s ease-out;
      display: flex;
      align-items: center;
      justify-content: center;
    }
    .led-core {
      width: 40px;
      height: 40px;
      border-radius: 50%;
      background: #1a1a2e;
      box-shadow: inset 0 2px 10px rgba(0,0,0,0.5);
    }
    .value-wrapper {
      margin: 25px 0;
    }
    .brightness-display {
      font-size: 64px;
      font-weight: 700;
      color: #00d4ff;
      line-height: 1;
      text-shadow: 0 0 30px rgba(0,212,255,0.3);
      transition: all 0.05s;
    }
    .brightness-label {
      font-size: 12px;
      color: rgba(255,255,255,0.4);
      margin-top: 8px;
      letter-spacing: 2px;
      text-transform: uppercase;
    }
    .slider-wrapper {
      margin: 20px 0;
      padding: 0 5px;
    }
    .slider-track {
      position: relative;
      height: 8px;
      border-radius: 4px;
      background: rgba(255,255,255,0.1);
      overflow: visible;
    }
    .slider-fill {
      position: absolute;
      left: 0;
      top: 0;
      height: 100%;
      border-radius: 4px;
      background: linear-gradient(90deg, #00d4ff, #7b2ff7);
      width: 0%;
      transition: width 0.05s;
      pointer-events: none;
    }
    input[type="range"] {
      -webkit-appearance: none;
      appearance: none;
      width: 100%;
      height: 8px;
      border-radius: 4px;
      background: transparent;
      outline: none;
      cursor: pointer;
      position: relative;
      z-index: 2;
      margin-top: -8px;
      display: block;
    }
    input[type="range"]::-webkit-slider-thumb {
      -webkit-appearance: none;
      width: 32px;
      height: 32px;
      border-radius: 50%;
      background: linear-gradient(135deg, #00d4ff, #7b2ff7);
      cursor: pointer;
      box-shadow: 0 0 20px rgba(0,212,255,0.5), 0 4px 10px rgba(0,0,0,0.3);
      border: 3px solid #fff;
      transition: transform 0.1s;
    }
    input[type="range"]::-webkit-slider-thumb:active {
      transform: scale(1.15);
    }
    input[type="range"]::-moz-range-thumb {
      width: 32px;
      height: 32px;
      border-radius: 50%;
      background: linear-gradient(135deg, #00d4ff, #7b2ff7);
      cursor: pointer;
      box-shadow: 0 0 20px rgba(0,212,255,0.5), 0 4px 10px rgba(0,0,0,0.3);
      border: 3px solid #fff;
    }
    .percent-markers {
      display: flex;
      justify-content: space-between;
      margin-top: 10px;
      padding: 0 8px;
      font-size: 11px;
      color: rgba(255,255,255,0.3);
    }
    .input-box-area {
      margin: 30px 0;
      display: flex;
      gap: 10px;
      justify-content: center;
      align-items: center;
    }
    #numInput {
      width: 120px;
      height: 44px;
      font-size: 18px;
      text-align: center;
      border-radius: 12px;
      border: 1px solid rgba(0,212,255,0.3);
      background: rgba(255,255,255,0.08);
      color: #fff;
      outline: none;
      padding: 0 10px;
    }
    #numInput:focus {
      border-color: #00d4ff;
      box-shadow: 0 0 12px rgba(0,212,255,0.25);
    }
    #confirmBtn {
      height: 44px;
      padding: 0 22px;
      border-radius: 12px;
      border: none;
      background: linear-gradient(90deg, #00d4ff, #7b2ff7);
      color: white;
      font-size: 15px;
      cursor: pointer;
      transition: opacity 0.2s;
    }
    #confirmBtn:hover {
      opacity: 0.85;
    }
    .status-bar {
      margin-top: 25px;
      padding: 12px 20px;
      border-radius: 12px;
      background: rgba(0,212,255,0.08);
      border: 1px solid rgba(0,212,255,0.15);
      font-size: 13px;
      color: rgba(0,212,255,0.8);
      letter-spacing: 0.5px;
      transition: all 0.3s;
    }
    .status-bar.offline {
      color: #ff6b6b;
      background: rgba(255,107,107,0.08);
      border-color: rgba(255,107,107,0.15);
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>无极调光器</h1>
    <p class="subtitle">ESP32 AP热点 PWM实时控制</p>
    
    <div class="led-ring" id="ledRing">
      <div class="led-core" id="ledCore"></div>
    </div>
    
    <div class="value-wrapper">
      <div class="brightness-display" id="brightnessValue">0</div>
      <div class="brightness-label">Brightness</div>
    </div>

    <!-- 数字输入框区域 -->
    <div class="input-box-area">
      <input type="number" id="numInput" min="0" max="255" placeholder="输入0-255">
      <button id="confirmBtn">确认</button>
    </div>
    
    <div class="slider-wrapper">
      <div class="slider-track">
        <div class="sliderFill" id="sliderFill"></div>
      </div>
      <input type="range" id="brightnessSlider" min="0" max="255" value="0">
      <div class="percent-markers">
        <span>0</span>
        <span>64</span>
        <span>128</span>
        <span>192</span>
        <span>255</span>
      </div>
    </div>
    
    <div class="status-bar" id="status">连接热点 ESP32_Dimmer，访问 192.168.4.1</div>
  </div>

  <script>
    (function() {
      const slider = document.getElementById('brightnessSlider');
      const valueDisplay = document.getElementById('brightnessValue');
      const statusText = document.getElementById('status');
      const ledRing = document.getElementById('ledRing');
      const ledCore = document.getElementById('ledCore');
      const sliderFill = document.getElementById('sliderFill');
      const numInput = document.getElementById('numInput');
      const confirmBtn = document.getElementById('confirmBtn');

      let lastSentValue = -1;
      let pendingValue = -1;
      let sendTimer = null;

      // 更新所有页面视觉
      function updateVisuals(value) {
        value = Math.max(0, Math.min(255, value));
        const ratio = value / 255;
        
        valueDisplay.textContent = value;
        slider.value = value;
        numInput.value = value;
        sliderFill.style.width = (ratio * 100) + '%';
        
        // LED光环发光
        const glowIntensity = 0.1 + ratio * 0.9;
        ledRing.style.borderColor = 'rgba(0,212,255,' + (0.2 + ratio * 0.8) + ')';
        ledRing.style.boxShadow = '0 0 ' + (20 + ratio * 60) + 'px rgba(0,212,255,' + glowIntensity + ')';
        ledRing.style.background = 'radial-gradient(circle, rgba(0,212,255,' + (0.1 + ratio * 0.5) + ') 0%, transparent 70%)';
        
        // LED内核
        ledCore.style.background = ratio > 0.05 
          ? 'linear-gradient(135deg, rgba(0,212,255,' + ratio + '), rgba(123,47,247,' + ratio + '))'
          : '#1a1a2e';
        ledCore.style.boxShadow = ratio > 0.05
          ? '0 0 ' + (10 + ratio * 30) + 'px rgba(0,212,255,' + ratio + ')'
          : 'inset 0 2px 10px rgba(0,0,0,0.5)';
      }

      // 发送亮度到ESP32
      function doSend(value) {
        if (value === lastSentValue) return;
        lastSentValue = value;
        
        const img = new Image();
        img.onload = function() {
          statusText.textContent = '亮度: ' + value + ' / 255';
          statusText.classList.remove('offline');
        };
        img.onerror = function() {
          statusText.textContent = '发送失败，请重新连接热点';
          statusText.classList.add('offline');
        };
        img.src = '/set?brightness=' + value + '&t=' + Date.now();
      }

      function sendBrightness(value) {
        pendingValue = value;
        if (sendTimer) clearTimeout(sendTimer);
        doSend(value);
        sendTimer = setTimeout(function() {
          if (pendingValue !== lastSentValue) doSend(pendingValue);
          sendTimer = null;
        }, 30);
      }

      // 滑块拖动事件
      slider.addEventListener('input', function() {
        let val = parseInt(this.value);
        updateVisuals(val);
        sendBrightness(val);
      });
      slider.addEventListener('change', function() {
        let val = parseInt(this.value);
        updateVisuals(val);
        doSend(val);
      });

      // 输入框确认按钮
      confirmBtn.addEventListener('click', function() {
        let val = parseInt(numInput.value);
        if (isNaN(val)) val = 0;
        val = Math.max(0, Math.min(255, val));
        updateVisuals(val);
        doSend(val);
      });

      // 输入框回车快捷确认
      numInput.addEventListener('keydown', function(e) {
        if (e.key === 'Enter') {
          confirmBtn.click();
        }
      });

      // 页面初始化
      updateVisuals(0);
    })();
  </script>
</body>
</html>
)rawliteral";
  return html;
}

void handleRoot() {
  server.send(200, "text/html; charset=UTF-8", makePage());
}

void handleSetBrightness() {
  if (server.hasArg("brightness")) {
    int brightness = server.arg("brightness").toInt();
    brightness = constrain(brightness, 0, 255);
    ledcWrite(LED_PIN, brightness);
    server.send(200, "image/gif", "R0lGODlhAQABAIAAAAAAAP///yH5BAEAAAAALAAAAAABAAEAAAIBRAA7");
  } else {
    server.send(400, "text/plain", "Missing parameter");
  }
}

void setup() {
  Serial.begin(115200);
  
  // PWM初始化LED
  ledcAttach(LED_PIN, PWM_FREQ, PWM_RESOLUTION);
  ledcWrite(LED_PIN, 0);

  // 关闭STA模式，只开启AP热点
  WiFi.mode(WIFI_AP);
  // 创建热点
  WiFi.softAP(ap_ssid, ap_password);

  IPAddress apIP = WiFi.softAPIP();
  Serial.println("======== ESP32 AP热点启动 ========");
  Serial.print("热点名称：");
  Serial.println(ap_ssid);
  Serial.print("热点密码：");
  Serial.println(ap_password);
  Serial.print("访问地址：http://");
  Serial.println(apIP);

  // 注册网页路由
  server.on("/", handleRoot);
  server.on("/set", handleSetBrightness);
  server.begin();
  Serial.println("Web服务器运行中...");
}

void loop() {
  server.handleClient();
}