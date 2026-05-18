#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <BH1750.h>
#include <ArduinoJson.h>

const char* ssid = "linh123";
const char* password = "12345678";
const char* tb_server = "eu.thingsboard.cloud"; 
const char* tb_token = "Vr0E8EIYgWFMb2lei75i"; 

WiFiClient espClient;
PubSubClient client(espClient);
BH1750 lightMeter;
bool hasBH1750 = false; 

#define START_BYTE  0xAA
#define END_BYTE    0x55
#define CMD_TEXT    0x01
#define CMD_ACK     0x02

bool isAutoMode = false;      
bool manualLedState = false;  
int tempThreshold = 29;       
bool currentLedState = false; 

int current_lux = 0;
int stm_temp = 0;
int stm_hum = 0;
unsigned long lastSend = 0;

typedef struct {
    uint8_t start;
    uint8_t cmd;
    uint8_t len;
    uint8_t data[64];
    uint8_t checksum;
    uint8_t end;
} Message_t;

uint8_t MSG_Checksum(Message_t *msg) {
    uint8_t cs = msg->cmd ^ msg->len;
    for (int i = 0; i < msg->len; i++) cs ^= msg->data[i];
    return cs;
}

void SendToSTM32(uint8_t cmd, const char* payload) {
    Message_t msg;
    msg.start = START_BYTE;
    msg.cmd = cmd;
    msg.len = strlen(payload);
    memcpy(msg.data, payload, msg.len);
    msg.checksum = MSG_Checksum(&msg);
    msg.end = END_BYTE;

    Serial2.write(msg.start);
    Serial2.write(msg.cmd);
    Serial2.write(msg.len);
    for (int i = 0; i < msg.len; i++) Serial2.write(msg.data[i]);
    Serial2.write(msg.checksum);
    Serial2.write(msg.end);
}

// ==========================================
// HÀM BẮT LỆNH & GỬI BIÊN LAI (RPC) 
// ==========================================
void on_message(char* topic, byte* payload, unsigned int length) {
    char json[length + 1];
    strncpy(json, (char*)payload, length);
    json[length] = '\0';
    Serial.print("Nhận lệnh từ Web: ");
    Serial.println(json);

    // 1. Tách lấy mã ID của gói tin để gửi biên lai trả lời đúng người
    String topicStr = String(topic);
    int reqIdIndex = topicStr.lastIndexOf('/') + 1;
    String requestId = topicStr.substring(reqIdIndex);

    // 2. Mổ xẻ JSON
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error) return;

    String method = doc["method"].as<String>();
    
    // 3. Xử lý các lệnh
    if (method == "setMode") {
        isAutoMode = doc["params"].as<bool>();
        Serial.printf("=> Đã chuyển sang chế độ: %s\n", isAutoMode ? "AUTO" : "MANUAL");
    }
    else if (method == "setLed") {
        manualLedState = doc["params"].as<bool>();
        if (!isAutoMode) {
            Serial.printf("=> Bấm Web (Thủ công): %s đèn\n", manualLedState ? "BẬT" : "TẮT");
        }
    }
    else if (method == "setThreshold") {
        tempThreshold = doc["params"].as<int>();
        Serial.printf("=> Ngưỡng tự động mới: %d độ C\n", tempThreshold);
    }
    else if (method == "getState") {
        // Web hỏi trạng thái lúc mới tải trang -> Không cần in ra, chỉ cần trả lời
    }

    // 4. GỬI BIÊN LAI PHẢN HỒI LẠI CHO WEB ĐỂ XÓA LỖI TIMEOUT
    String responseTopic = "v1/devices/me/rpc/response/" + requestId;
    // Trả về trạng thái đèn hiện tại
    String responsePayload = String(currentLedState ? "true" : "false"); 
    client.publish(responseTopic.c_str(), responsePayload.c_str());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Ket noi ThingsBoard...");
    if (client.connect("ESP32_Node1", tb_token, NULL)) {
      Serial.println(" OK!");
      client.subscribe("v1/devices/me/rpc/request/+");
    } else {
      delay(3000);
    }
  }
}

void setup() {
  Serial.begin(115200);   
  Serial2.begin(115200, SERIAL_8N1, 16, 17); 
  Serial2.setTimeout(100); 
  
  Wire.begin();
  if (lightMeter.begin()) {
      Serial.println("Đã tìm thấy cảm biến BH1750!");
      hasBH1750 = true;
  } else {
      Serial.println("CẢNH BÁO: Không tìm thấy BH1750, chạy giả lập Lux!");
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("\nWiFi OK!");

  client.setServer(tb_server, 1883);
  client.setCallback(on_message);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  // 1. LẮNG NGHE STM32 (BẢN CHỐNG TREO)
  if (Serial2.available() > 0) {
    if (Serial2.read() == START_BYTE) {
      Message_t rx_msg;
      rx_msg.start = START_BYTE;
      
      byte header[2];
      if (Serial2.readBytes(header, 2) == 2) {
          rx_msg.cmd = header[0];
          rx_msg.len = header[1];
          
          if (Serial2.readBytes(rx_msg.data, rx_msg.len) == rx_msg.len) {
              rx_msg.data[rx_msg.len] = '\0';
              
              byte tail[2];
              if (Serial2.readBytes(tail, 2) == 2) {
                  rx_msg.checksum = tail[0];
                  rx_msg.end = tail[1];
                  
                  if (rx_msg.end == END_BYTE && rx_msg.checksum == MSG_Checksum(&rx_msg)) {
                      if (rx_msg.cmd == CMD_TEXT) {
                          Serial.printf(">> Nhận từ STM32: %s\n", rx_msg.data);
                          sscanf((char*)rx_msg.data, "Phan hoi tu Node 2 ve Node 1: T:%dC H:%d%%", &stm_temp, &stm_hum);
                      }
                  }
              }
          }
      }
    }
  }

  // 2. XỬ LÝ LOGIC ĐIỀU KHIỂN & GỬI LỆNH SANG STM32
  bool targetLedState = currentLedState;
  if (isAutoMode) {
      targetLedState = (stm_temp > tempThreshold);
  } else {
      targetLedState = manualLedState;
  }

  if (targetLedState != currentLedState) {
      currentLedState = targetLedState;
      if (currentLedState) {
          SendToSTM32(CMD_ACK, "LED_ON");
          Serial.println("=> Gửi lệnh UART: BẬT ĐÈN STM32");
      } else {
          SendToSTM32(CMD_ACK, "LED_OFF");
          Serial.println("=> Gửi lệnh UART: TẮT ĐÈN STM32");
      }
      String stateJson = "{\"ledStatus\":" + String(currentLedState ? "true" : "false") + "}";
      client.publish("v1/devices/me/telemetry", stateJson.c_str());
  }

  // 3. ĐẨY DỮ LIỆU LÊN THINGSBOARD & STM32 MỖI 2 GIÂY
  if (millis() - lastSend > 2000) {
    lastSend = millis();
    
    if (hasBH1750) {
        current_lux = lightMeter.readLightLevel();
    } else {
        current_lux = random(50, 200); 
    }

    String telemetry = "{\"lux\":" + String(current_lux) + 
                       ", \"nhietdo\":" + String(stm_temp) + 
                       ", \"doam\":" + String(stm_hum) + 
                       ", \"isAuto\":" + String(isAutoMode ? "true" : "false") + 
                       ", \"nguong\":" + String(tempThreshold) + "}";
    client.publish("v1/devices/me/telemetry", telemetry.c_str());
    
    Serial.println("Đã đẩy lên ThingsBoard: " + telemetry);
    
    // GỬI LỆNH ĐÁNH THỨC STM32
    char tx_buf[64];
    sprintf(tx_buf, "Linh - DT177 - %d lux", current_lux);
    SendToSTM32(CMD_TEXT, tx_buf);
  }
}