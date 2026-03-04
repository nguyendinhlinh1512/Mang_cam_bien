#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>


#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; 

unsigned long delayTime;

void printValues() {
  Serial.print("Temperature = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");
  
  
  Serial.print("Humidity = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");

  Serial.println();
}

void setup() {
  Serial.begin(9600);
  Serial.println(F("BME280 test")); //In thông báo ( lưu chuỗi vào Flash thay vì RAM ,F() giúp tiết kiệm RAM)

  bool status; 

  status = bme.begin(0x76);   //  địa chỉ I2C 0x76.
  if (!status) { 
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1); //  dừng chương trình
  }

  Serial.println("-- Default Test --");
  delayTime = 1000;

  Serial.println();
}

void loop() { 
  printValues();
  delay(delayTime);
}

