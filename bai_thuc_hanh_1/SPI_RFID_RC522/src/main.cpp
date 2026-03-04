#include <Arduino.h>
#include <SPI.h>          
#include <MFRC522.h>      

#define SS_PIN 5          
#define RST_PIN 0         // Chân Reset của module


MFRC522 rfid(SS_PIN, RST_PIN);


MFRC522::MIFARE_Key key;

// Mảng lưu UID của thẻ đã quét trước 
byte nuidPICC[4];


void printHex(byte *buffer, byte bufferSize);
void printDec(byte *buffer, byte bufferSize);

void setup()
{
    Serial.begin(9600);   
    SPI.begin();          
    rfid.PCD_Init();      

   
    for (byte i = 0; i < 6; i++)
    {
        key.keyByte[i] = 0xFF;
    }

    Serial.println(F("Scan MIFARE Classic NUID"));
    Serial.print(F("Using key: "));
    printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
    Serial.println();
}

void loop()
{

    if (!rfid.PICC_IsNewCardPresent())
        return;


    if (!rfid.PICC_ReadCardSerial())
        return;

    // Lấy loại thẻ
    Serial.print(F("PICC type: "));
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    Serial.println(rfid.PICC_GetTypeName(piccType));

    // Kiểm tra có phải thẻ MIFARE Classic không 
    if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
        piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
        piccType != MFRC522::PICC_TYPE_MIFARE_4K)
    {
        Serial.println(F("Not MIFARE Classic"));
        return;
    }

    // So sánh UID hiện tại với UID đã lưu trước đó
    if (rfid.uid.uidByte[0] != nuidPICC[0] ||
        rfid.uid.uidByte[1] != nuidPICC[1] ||
        rfid.uid.uidByte[2] != nuidPICC[2] ||
        rfid.uid.uidByte[3] != nuidPICC[3])
    {
        Serial.println(F("New card detected"));

        // Lưu UID mới
        for (byte i = 0; i < 4; i++)
        {
            nuidPICC[i] = rfid.uid.uidByte[i];
        }

        // In UID dạng HEX
        Serial.print(F("UID (HEX): "));
        printHex(rfid.uid.uidByte, rfid.uid.size);
        Serial.println();

        // In UID dạng DEC
        Serial.print(F("UID (DEC): "));
        printDec(rfid.uid.uidByte, rfid.uid.size);
        Serial.println();
    }
    else
    {
        Serial.println(F("Card already read"));
    }

    // Kết thúc giao tiếp với thẻ Yêu cầu thẻ ngừng truyền Tắt mã hóa
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
}

// Hàm in dữ liệu dạng HEX
void printHex(byte *buffer, byte bufferSize)
{
    for (byte i = 0; i < bufferSize; i++)
    {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], HEX);
    }
}

// Hàm in dữ liệu dạng thập phân
void printDec(byte *buffer, byte bufferSize)
{
    for (byte i = 0; i < bufferSize; i++)
    {
        Serial.print(buffer[i] < 0x10 ? " 0" : " ");
        Serial.print(buffer[i], DEC);
    }
}