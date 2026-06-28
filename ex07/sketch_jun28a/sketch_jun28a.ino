#include <WiFi.h>
#include <WebServer.h>

// ========== WiFi 配置 ==========
const char* ssid = "unknown";
const char* password = "9230423lm";

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
      margin: 35px 0;
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
    .percent-markers {
      display: flex;
      justify-content: space-between;
      margin-top: 10px;
      padding: 0 8px;
      font-size: 11px;
      color: rgba(255,255,255,0.3);
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>无极调光器</h1>
    <p class="subtitle">ESP32 PWM 实时控制</p>
    
    <div class="led-ring" id="ledRing">
      <div class="led-core" id="ledCore"></div>
    </div>
    
    <div class="value-wrapper">
      <div class="brightness-display" id="brightnessValue">0</div>
      <div class="brightness-label">Brightness</div>
    </div>
    
    <div class="slider-wrapper">
      <div class="slider-track">
        <div class="slider-fill" id="sliderFill"></div>
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
    
    <div class="status-bar" id="status">就绪 - 拖动滑块调节亮度</div>
  </div>

  <script>
    (function() {
      const slider = document.getElementById('brightnessSlider');
      const valueDisplay = document.getElementById('brightnessValue');
      const statusText = document.getElementById('status');
      const ledRing = document.getElementById('ledRing');
      const ledCore = document.getElementById('ledCore');
      const sliderFill = document.getElementById('sliderFill');
      let lastSentValue = -1;
      let pendingValue = -1;
      let sendTimer = null;

      function updateVisuals(value) {
        const ratio = value / 255;
        
        // 数值显示
        valueDisplay.textContent = value;
        
        // 滑块填充条
        sliderFill.style.width = (ratio * 100) + '%';
        
        // LED光环
        const glowIntensity = 0.1 + ratio * 0.9;
        ledRing.style.borderColor = 'rgba(0,212,255,' + (0.2 + ratio * 0.8) + ')';
        ledRing.style.boxShadow = '0 0 ' + (20 + ratio * 60) + 'px rgba(0,212,255,' + glowIntensity + ')';
        ledRing.style.background = 'radial-gradient(circle, rgba(0,212,255,' + (0.1 + ratio * 0.5) + ') 0%, transparent 70%)';
        
        // LED核心
        ledCore.style.background = ratio > 0.05 
          ? 'linear-gradient(135deg, rgba(0,212,255,' + ratio + '), rgba(123,47,247,' + ratio + '))'
          : '#1a1a2e';
        ledCore.style.boxShadow = ratio > 0.05
          ? '0 0 ' + (10 + ratio * 30) + 'px rgba(0,212,255,' + ratio + ')'
          : 'inset 0 2px 10px rgba(0,0,0,0.5)';
      }

      function doSend(value) {
        if (value === lastSentValue) return;
        lastSentValue = value;
        
        // 使用 Image 对象实现无阻塞发送，兼容性最好
        const img = new Image();
        img.onload = function() {
          statusText.textContent = '亮度: ' + value + ' / 255';
          statusText.classList.remove('offline');
        };
        img.onerror = function() {
          statusText.textContent = '发送失败，请检查连接';
          statusText.classList.add('offline');
        };
        img.src = '/set?brightness=' + value + '&t=' + Date.now();
      }

      function sendBrightness(value) {
        pendingValue = value;
        
        // 取消之前的定时器
        if (sendTimer) {
          clearTimeout(sendTimer);
          sendTimer = null;
        }
        
        // 立即发送（如果距离上次发送超过30ms）
        doSend(value);
        
        // 如果还在快速拖动，用定时器补发最后一个值
        sendTimer = setTimeout(function() {
          if (pendingValue !== lastSentValue) {
            doSend(pendingValue);
          }
          sendTimer = null;
        }, 30);
      }

      // 监听 input 事件（实时拖动）
      slider.addEventListener('input', function() {
        var value = parseInt(this.value);
        updateVisuals(value);
        sendBrightness(value);
      });

      // 监听 change 事件（拖动结束，确保最终值送达）
      slider.addEventListener('change', function() {
        var value = parseInt(this.value);
        updateVisuals(value);
        doSend(value);
      });

      // 初始化
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
  
  ledcAttach(LED_PIN, PWM_FREQ, PWM_RESOLUTION);
  ledcWrite(LED_PIN, 0);
  
  WiFi.begin(ssid, password);
  Serial.print("连接WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\n连接成功");
  Serial.print("访问地址: http://");
  Serial.println(WiFi.localIP());
  
  server.on("/", handleRoot);
  server.on("/set", handleSetBrightness);
  
  server.begin();
  Serial.println("Web服务器已启动");
}

void loop() {
  server.handleClient();
}