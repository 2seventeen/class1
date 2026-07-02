#include <WiFi.h>
#include <WebServer.h>

// AP热点配置
const char* ap_ssid = "ESP32-LAB17";
const char* ap_password = "12345678";

// 三路LED引脚 红、黄、绿
const int PIN_RED   = 2;
const int PIN_YELLOW= 4;
const int PIN_GREEN = 5;
const int PWM_FREQ = 5000;
const int PWM_RES = 8; // 0~255

uint8_t briRed = 0, briYel = 0, briGre = 0;
WebServer server(80);

String makePage() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="zh-CN">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>红黄绿三色灯控制器</title>
<style>
*{margin:0;padding:0;box-sizing:border-box;font-family:Arial,Microsoft YaHei}
body{background:#111;color:#eee;padding:20px;max-width:500px;margin:0 auto}
h2{text-align:center;margin-bottom:6px;font-size:22px}
.tip{text-align:center;color:#888;font-size:13px;margin-bottom:25px}
.box{background:#1c1c1c;border:1px solid #333;border-radius:14px;padding:20px}
.channel{margin-bottom:22px;padding-bottom:20px;border-bottom:1px solid #333}
.channel:last-child{border:none;margin-bottom:0;padding-bottom:0}
.head{display:flex;justify-content:space-between;align-items:center;margin-bottom:12px}
.name{display:flex;align-items:center;gap:10px;font-size:16px}
.dot{width:16px;height:16px;border-radius:50%;background:#555}
.dot.r{border:1px solid #f33}
.dot.y{border:1px solid #fd0}
.dot.g{border:1px solid #3f3}
.val{font-size:18px;color:#0cf;width:60px;text-align:right}
.slider{width:100%;height:6px;background:#333;border-radius:3px;-webkit-appearance:none;outline:none}
.slider::-webkit-slider-thumb{-webkit-appearance:none;width:18px;height:18px;border-radius:50%;background:#0cf;cursor:pointer}
.inputRow{display:flex;gap:8px;margin-top:12px}
.num{flex:1;height:36px;background:#282828;border:1px solid #444;color:#fff;border-radius:8px;padding:0 10px;font-size:15px;outline:none}
.btn{height:36px;padding:0 16px;border:none;border-radius:8px;background:#0cf;color:#000;cursor:pointer}
.status{margin-top:22px;padding:12px;text-align:center;background:#222;border-radius:8px;font-size:13px;color:#aaa}
.status.err{background:#301a1a;color:#f66}
</style>
</head>
<body>
<h2>三色灯光控制</h2>
<p class="tip">热点:ESP32_RYG_Light | 地址192.168.4.1</p>
<div class="box">
  <!-- 红灯通道 -->
  <div class="channel">
    <div class="head">
      <div class="name"><span class="dot r" id="d1"></span>红灯(GPIO2)</div>
      <div class="val" id="v1">0</div>
    </div>
    <input class="slider" id="s1" type="range" min="0" max="255" value="0">
    <div class="inputRow">
      <input class="num" id="n1" type="number" min="0" max="255" placeholder="0~255">
      <button onclick="setNum(1)">确认</button>
    </div>
  </div>

  <!-- 黄灯通道 -->
  <div class="channel">
    <div class="head">
      <div class="name"><span class="dot y" id="d2"></span>黄灯(GPIO4)</div>
      <div class="val" id="v2">0</div>
    </div>
    <input class="slider" id="s2" type="range" min="0" max="255" value="0">
    <div class="inputRow">
      <input class="num" id="n2" type="number" min="0" max="255" placeholder="0~255">
      <button onclick="setNum(2)">确认</button>
    </div>
  </div>

  <!-- 绿色通道 -->
  <div class="channel">
    <div class="head">
      <div class="name"><span class="dot g" id="d3"></span>绿灯(GPIO5)</div>
      <div class="val" id="v3">0</div>
    </div>
    <input class="slider" id="s3" type="range" min="0" max="255" value="0">
    <div class="inputRow">
      <input class="num" id="n3" type="number" min="0" max="255" placeholder="0~255">
      <button onclick="setNum(3)">确认</button>
    </div>
  </div>
</div>
<div class="status" id="sta">等待滑动调节亮度</div>

<script>
let lastVal = [-1,-1,-1];
let timer = null;
const staDom = document.getElementById("sta");
const colorSet = {
  1:{low:"#600",mid:"#c22",high:"#f33"},
  2:{low:"#650",mid:"#ca0",high:"#fd0"},
  3:{low:"#060",mid:"#2c2",high:"#3f3"}
};

// 更新页面显示
function updateUI(ch, val){
  val = Math.max(0,Math.min(255,val));
  let r = val / 255;
  document.getElementById("v"+ch).innerText = val;
  document.getElementById("s"+ch).value = val;
  document.getElementById("n"+ch).value = val;
  let dot = document.getElementById("d"+ch);
  let c = colorSet[ch];
  if(r === 0){
    dot.style.background = "#555";
    dot.style.boxShadow = "none";
  }else if(r < 0.4){
    dot.style.background = c.low;
    dot.style.boxShadow = "none";
  }else{
    dot.style.background = c.mid;
    dot.style.boxShadow = "0 0 6px "+c.high;
  }
}

// 发送亮度到ESP32
function sendData(ch, val){
  if(lastVal[ch-1] === val) return;
  lastVal[ch-1] = val;
  let img = new Image();
  img.onload = ()=>{
    staDom.innerText = `通道${ch} 亮度:${val}`;
    staDom.classList.remove("err");
  }
  img.onerror = ()=>{
    staDom.innerText = "连接断开，请重连热点";
    staDom.classList.add("err");
  }
  img.src = `/set?ch=${ch}&v=${val}&t=${Date.now()}`;
}

// 滑块拖动事件
function slide(ch, val){
  updateUI(ch, val);
  clearTimeout(timer);
  sendData(ch, val);
  timer = setTimeout(()=>sendData(ch,val),25);
}

// 数字输入设置
function setNum(ch){
  let num = parseInt(document.getElementById("n"+ch).value);
  num = isNaN(num) ? 0 : Math.max(0,Math.min(255,num));
  updateUI(ch, num);
  sendData(ch, num);
}

// 绑定滑块
document.getElementById("s1").addEventListener("input",e=>slide(1,e.target.value));
document.getElementById("s2").addEventListener("input",e=>slide(2,e.target.value));
document.getElementById("s3").addEventListener("input",e=>slide(3,e.target.value));
// 回车确认
document.getElementById("n1").addEventListener("keydown",e=>e.key=="Enter"&&setNum(1));
document.getElementById("n2").addEventListener("keydown",e=>e.key=="Enter"&&setNum(2));
document.getElementById("n3").addEventListener("keydown",e=>e.key=="Enter"&&setNum(3));

// 初始清零
updateUI(1,0);
updateUI(2,0);
updateUI(3,0);
</script>
</body>
</html>
)rawliteral";
  return html;
}

// 首页页面
void handleRoot() {
  server.send(200, "text/html; charset=utf-8", makePage());
}

// 接收亮度指令
void handleSet() {
  if(!server.hasArg("ch") || !server.hasArg("v")){
    server.send(400, "text/plain", "参数缺失");
    return;
  }
  int ch = server.arg("ch").toInt();
  int v = constrain(server.arg("v").toInt(),0,255);
  switch(ch){
    case 1: briRed = v; ledcWrite(PIN_RED, briRed); break;
    case 2: briYel = v; ledcWrite(PIN_YELLOW, briYel); break;
    case 3: briGre = v; ledcWrite(PIN_GREEN, briGre); break;
    default: server.send(400,"text/plain","无效通道");return;
  }
  // 返回空白1px图片
  server.send(200, "image/gif", "R0lGODlhAQABAIAAAAAAAP///yH5BAEAAAAALAAAAAABAAEAAAIBRAA7");
}

void setup() {
  Serial.begin(115200);
  // 初始化三路PWM
  ledcAttach(PIN_RED, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_YELLOW, PWM_FREQ, PWM_RES);
  ledcAttach(PIN_GREEN, PWM_FREQ, PWM_RES);
  ledcWrite(PIN_RED, 0);
  ledcWrite(PIN_YELLOW, 0);
  ledcWrite(PIN_GREEN, 0);

  // 开启AP热点
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ap_ssid, ap_password);
  IPAddress apIp = WiFi.softAPIP();
  Serial.println("==== ESP32 三色灯AP启动 ====");
  Serial.print("热点名称:");Serial.println(ap_ssid);
  Serial.print("热点密码:");Serial.println(ap_password);
  Serial.print("访问地址:http://");Serial.println(apIp);
  Serial.println("红灯PIN2 | 黄灯PIN4 | 绿灯PIN5");

  // 注册接口
  server.on("/", handleRoot);
  server.on("/set", handleSet);
  server.begin();
  Serial.println("网页服务运行完成");
}

void loop() {
  server.handleClient();
}