#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>

// MAC address của Node 1
uint8_t node1Address[] = {0xFC, 0xB4, 0x67, 0xF1, 0xE8, 0x48}; 

const uint8_t MPU_ADDR = 0x68; 

// --- 1. STRUCT GỬI (Gia tốc bắn sang Node 1) --- 
typedef struct {
    char id[20];
    float accelX;
    float accelY;
    float accelZ;
} MPUData_t;

// --- 2. STRUCT NHẬN (Thông báo từ Node 1 bắn về) --- 
typedef struct {
    char info[80];
} Node1_Data_t;

MPUData_t myData; 
Node1_Data_t incomingData; 
esp_now_peer_info_t peerInfo; 

// HÀM BÁO CÁO TRẠNG THÁI GỬI 
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status != ESP_NOW_SEND_SUCCESS) {
        Serial.println("[Loi] Gui ESP-NOW That bai!");
    }
}

// NHẬN DỮ LIỆU TỪ NODE 1 
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingDataPtr, int len) {
    memcpy(&incomingData, incomingDataPtr, sizeof(incomingData));
    
    // In ra màn hình thông tin nhận được (chính là chuỗi Lux)
    Serial.print("<<< [NHAN TU NODE 1]: ");
    Serial.println(incomingData.info);
}

void setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);
    delay(500);

    
    Wire.beginTransmission(MPU_ADDR); 
    Wire.write(0x6B); 
    Wire.write(0);    // bo sleep
    if (Wire.endTransmission(true) != 0) {
        Serial.println(">>> LOI: KHONG TIM THAY MPU!");
        while(1);
    }
    Serial.println(">>> MPU SAN SANG!");

    WiFi.mode(WIFI_STA);
    if (esp_now_init() != ESP_OK) return;

    //  ĐĂNG KÝ 2 CHỨC NĂNG: GỬI, NHẬN 
    esp_now_register_send_cb(OnDataSent);
    esp_now_register_recv_cb(OnDataRecv); 
    
    memcpy(peerInfo.peer_addr, node1Address, 6);
    peerInfo.channel = 0;  
    peerInfo.encrypt = false;
    esp_now_add_peer(&peerInfo);

    strcpy(myData.id, "Node 3 (MPU)");
}

void loop() {
    // Đọc gia tốc thô
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x3B);  // thghi chua gia toc truc X
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, (size_t)6, true);  //tu dong tra du 6byte cua 3 truc

    int16_t ax = (Wire.read() << 8 | Wire.read());  
    int16_t ay = (Wire.read() << 8 | Wire.read());  
    int16_t az = (Wire.read() << 8 | Wire.read());  

    // Tính ra g
    myData.accelX = (ax / 16384.0f)*9.81; 
    myData.accelY = (ay / 16384.0f)*9.81; 
    myData.accelZ = (az / 16384.0f)*9.81; 

    Serial.printf(">>> [GUI GIA TOC] X: %.2f | Y: %.2f | Z: %.2f(m/s^2)\n", myData.accelX, myData.accelY, myData.accelZ);

    // Bắn sang Node 1
    esp_now_send(node1Address, (uint8_t *) &myData, sizeof(myData));
    
    delay(1000); // Gửi mỗi giây
}