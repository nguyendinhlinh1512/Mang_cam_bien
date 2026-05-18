#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <esp_now.h>
#include <BH1750.h>

BH1750 lightMeter;

#define LED_PIN 2
#define UART_RX_PIN 16
#define UART_TX_PIN 17

// --- 1. STRUCT NHẬN (Từ Node 3) ---
typedef struct {
    char id[20];
    float accelX;
    float accelY;
    float accelZ;
} MPUData_t;
MPUData_t incomingMPU;

// --- 2. STRUCT GỬI (Bắn sang Node 3) ---
typedef struct {
    char info[80];
} Node1_Data_t;
Node1_Data_t outDataToNode3;

// --- 3. BIẾN AUTO-PAIRING (Lưu địa chỉ MAC của Node 3) ---
esp_now_peer_info_t peerInfoNode3;
bool isNode3Connected = false;

// --- GIAO THỨC UART VỚI STM32 ---
#define START_BYTE 0xAA
#define END_BYTE   0x55
#define CMD_TEXT   0x01
#define CMD_ACK    0x02

typedef struct {
  uint8_t start;
  uint8_t cmd;
  uint8_t len;
  uint8_t data[65];
  uint8_t checksum;
  uint8_t end;
} Message_t;

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
    // Nhận gia tốc
    memcpy(&incomingMPU, incomingData, sizeof(incomingMPU));
    Serial.printf(">>> [FROM %s] X:%.2f | Y:%.2f | Z:%.2f(m/s^2)\n", 
                  incomingMPU.id, incomingMPU.accelX, incomingMPU.accelY, incomingMPU.accelZ);

    // --- TỰ ĐỘNG LƯU MAC CỦA NODE 3 ĐỂ GỬI NGƯỢC LẠI ---
    if (!isNode3Connected) {
        memcpy(peerInfoNode3.peer_addr, mac, 6);
        peerInfoNode3.channel = 0;
        peerInfoNode3.encrypt = false;
        if (esp_now_add_peer(&peerInfoNode3) == ESP_OK) {
            isNode3Connected = true;
            Serial.println("\n>> [ESP-NOW] DA KET NOI 2 CHIEU VOI NODE 3!\n");
        }
    }
}

// --- HÀM BỔ TRỢ UART ---
uint8_t MSG_Checksum(Message_t *msg) {
  uint8_t cs = msg->cmd ^ msg->len;
  for (int i = 0; i < msg->len; i++) cs ^= msg->data[i];
  return cs;
}

void MSG_Send_STM32(uint8_t cmd, const char *text) {
  Message_t msg;
  msg.start = START_BYTE; 
  msg.cmd = cmd; msg.len = (uint8_t)strlen(text);
  memcpy(msg.data, text, msg.len);
  msg.checksum = MSG_Checksum(&msg); 
  msg.end = END_BYTE;
  Serial2.write(msg.start); 
  Serial2.write(msg.cmd); 
  Serial2.write(msg.len);
  for (int i = 0; i < msg.len; i++) Serial2.write(msg.data[i]);
  Serial2.write(msg.checksum); 
  Serial2.write(msg.end);
}

static uint8_t rx_buf[128]; static uint8_t rx_idx = 0;
bool MSG_Receive_STM32(Message_t *out) {
  while (Serial2.available()) {
    uint8_t b = (uint8_t)Serial2.read();
    if (rx_idx == 0 && b != START_BYTE) continue;
    rx_buf[rx_idx++] = b;
    if (rx_idx >= 3) {
      uint8_t total = 3 + rx_buf[2] + 2;
      if (rx_idx == total) {
        out->cmd = rx_buf[1]; out->len = rx_buf[2];
        memcpy(out->data, &rx_buf[3], out->len); out->data[out->len] = 0;
        out->checksum = rx_buf[3 + out->len]; out->end = rx_buf[3 + out->len + 1];
        rx_idx = 0; return (out->end == END_BYTE);
      }
    }
    if (rx_idx >= 128) rx_idx = 0;
  }
  return false;
}

unsigned long lastSend = 0;

void setup() {
  Serial.begin(115200);
  Serial2.begin(115200, SERIAL_8N1, UART_RX_PIN, UART_TX_PIN);
  
  Wire.begin(21, 22);
  delay(1000);
  
  if (!lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE)) {
    Serial.println(">>> LOI: KHONG TIM THAY BH1750!");
  }

  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); 
  if (esp_now_init() != ESP_OK) return;
  
  esp_now_register_recv_cb(OnDataRecv);
  
  pinMode(LED_PIN, OUTPUT);
  Serial.println(">>> ESP32 NODE 1 READY!");
}

void loop() {
  Message_t rx_msg;
  
  if (millis() - lastSend > 1000) {
    uint16_t lux = (uint16_t)lightMeter.readLightLevel();
    char payload[64];
    snprintf(payload, sizeof(payload), "Linh - DT177 - %d lux", lux);
    
    // 1. Gửi sang STM32
    MSG_Send_STM32(CMD_TEXT, payload);
    Serial.printf("[TX->STM32] %s\n", payload);

    // --- 2. BẮN NGƯỢC SANG NODE 3 (NẾU ĐÃ AUTO-PAIR) ---
    if (isNode3Connected) {
        strcpy(outDataToNode3.info, payload);
        esp_now_send(peerInfoNode3.peer_addr, (uint8_t *)&outDataToNode3, sizeof(outDataToNode3));
    }

    lastSend = millis();
  }

  if (MSG_Receive_STM32(&rx_msg)) {
    if (rx_msg.cmd == CMD_TEXT) {
      Serial.printf("[RX<-STM32] %s\n", (char*)rx_msg.data);
    }
    else if (rx_msg.cmd == CMD_ACK) { 
      Serial.printf("[LENH TU STM32] %s\n", (char*)rx_msg.data);
      if (strstr((char*)rx_msg.data, "LED_ON")) digitalWrite(LED_PIN, HIGH);
      else if (strstr((char*)rx_msg.data, "LED_OFF")) digitalWrite(LED_PIN, LOW);
    }
  }
}