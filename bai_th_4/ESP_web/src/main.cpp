#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ==========================================
// CẤU HÌNH PHẦN CỨNG (Sửa theo mạch thực tế)
// ==========================================
#define DHTPIN 4          // Chân Data của DHT11 nối với GPIO 4 của ESP32
#define DHTTYPE DHT11     // Khai báo loại cảm biến là DHT11
#define LED_PIN 2         // Chân LED cảnh báo (GPIO 2 là LED xanh tích hợp sẵn)
#define TEMP_THRESHOLD 31.8 // Ngưỡng nhiệt độ báo động 

DHT dht(DHTPIN, DHTTYPE);

// ==========================================
// THÔNG SỐ MẠNG & MQTT
// ==========================================
const char* ssid = "HienLinhh";      
const char* password = "88486917";     
const char* mqtt_server = "broker.hivemq.com"; 
const char* mqtt_topic_temp = "linh_dt177_esp32/nhietdo"; // Kênh gửi Nhiệt độ
const char* mqtt_topic_hum = "linh_dt177_esp32/doam";     // Kênh gửi Độ ẩm

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;

void setup_wifi() {
  delay(10);
  Serial.println("\nĐang kết nối Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nĐã kết nối Wi-Fi!");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Đang kết nối MQTT...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println(" THÀNH CÔNG!");
    } else {
      Serial.print(" LỖI, thử lại sau 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Khởi tạo phần cứng
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // Tắt LED ban đầu
  dht.begin();                // Khởi động cảm biến DHT11

  // Khởi tạo mạng
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  // Đọc DHT11 mỗi 2 giây (2000ms) - Bắt buộc để tránh lỗi NaN (Not a Number)
  if (now - lastMsg > 2000) {
    lastMsg = now;
    
    // 1. Quá trình lấy mẫu dữ liệu từ cảm biến
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    // Kiểm tra xem dữ liệu đọc về có bị lỗi nhiễu điện hay đứt dây không
    if (isnan(h) || isnan(t)) {
      Serial.println("Lỗi: Đứt dây hoặc hỏng DHT11!");
      
      // Bắn thẳng chữ "null" lên Node-RED
      client.publish(mqtt_topic_temp, "null");
      client.publish(mqtt_topic_hum, "null");
      
      // Tắt luôn đèn LED cho an toàn khi hệ thống lỗi
      digitalWrite(LED_PIN, LOW); 
      return; 
    }

    Serial.printf("Nhiệt độ: %.1f°C | Độ ẩm: %.1f%%\n", t, h);

    // 2. Quá trình xử lý Logic (So sánh ngưỡng)
    if (t > TEMP_THRESHOLD) {
      digitalWrite(LED_PIN, HIGH); // Cấp điện áp 3.3V ra chân LED
      Serial.println("=> VƯỢT NGƯỠNG: Đã BẬT LED cảnh báo!");
    } else {
      digitalWrite(LED_PIN, LOW);  // Kéo chân điện áp về 0V
    }

    // 3. Quá trình đóng gói và truyền tải lên Node-RED
    String tempStr = String(t, 1); // Cắt lấy 1 chữ số thập phân
    String humStr = String(h, 1);

    client.publish(mqtt_topic_temp, tempStr.c_str());
    client.publish(mqtt_topic_hum, humStr.c_str());
  }
}