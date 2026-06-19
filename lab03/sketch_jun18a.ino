// 定义LED引脚
const int ledPin1 = 4;  
const int ledPin2 = 2;

// 设置PWM属性
const int freq = 5000;          // 频率 5000Hz
const int resolution = 8;       // 分辨率 8位 (0-255)

void setup() {
  Serial.begin(115200);

  
  ledcAttach(ledPin1, freq, resolution);
  ledcAttach(ledPin2, freq, resolution);
}

void loop() {
  // 阶段1：LED1渐亮(0→255)，LED2同步渐暗(255→0)
  for(int duty = 0; duty <= 255; duty++){   
    ledcWrite(ledPin1, duty); 
    ledcWrite(ledPin2, 255 - duty);   
    delay(10);
  }

  // 阶段2：LED1渐暗(255→0)，LED2同步渐亮(0→255)
  for(int duty = 255; duty >= 0; duty--){
    ledcWrite(ledPin1, duty); 
    ledcWrite(ledPin2, 255 - duty);   
    delay(10);
  }
  
  Serial.println("反向呼吸一轮完成");
}