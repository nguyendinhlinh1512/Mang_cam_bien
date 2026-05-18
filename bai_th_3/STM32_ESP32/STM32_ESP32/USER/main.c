#include "stm32f10x.h"
#include "uart.h"
#include "tim2.h"
#include "dht11.h"
#include <string.h>

// =============================================================
//  Sinh vien: Linh - MSV: DT177
//  Node 2 : STM32 + DHT11 (PB12) + LED (PC13)
//  
//  Logic hoat dong (Che do Phan hoi):
//  1. Doi nhan ban tin tu ESP32 (Node 1) qua USART2
//  2. Khi nhan duoc:
//     - Parse gia tri Lux -> Dieu khien LED PC13 (Lux > 100 thi bat)
//     - Doc DHT11 -> Lay nhiet do, do am
//     - Gui ban tin phan hoi: "Phan hoi tu Node 2 ve Node 1: T:<temp>C H:<hum>%"
//     - Kiem tra nhiet do -> Gui lenh CMD_ACK dieu khien LED D2 tren ESP32
// =============================================================

#define TEMP_THRESHOLD  28   // Nguong nhiet do de bat LED ESP32
#define LUX_THRESHOLD   100  // Nguong anh sang de bat LED STM32

//  chuyen doi so thanh chuoi 
static void i16_to_str(char *buf, int16_t val)
{
    uint8_t i = 0;
    if (val == 0) { buf[0]='0'; buf[1]=0; return; }
    if (val < 0)  { buf[i++]='-'; val = -val; }
    char tmp[8]; uint8_t n = 0;
    while (val > 0) { tmp[n++] = '0' + (val % 10); val /= 10; }
    for (int8_t k = n-1; k >= 0; k--) buf[i++] = tmp[k];
    buf[i] = 0;
}

// ---- Ham parse gia tri Lux tu chuoi "Linh - DT177 - <lux> lux" ----
static int32_t parse_lux(const char *str) {
    const char *p = strstr(str, " - "); // Tìm d?u g?ch ngang d?u tiên
    if (p) p = strstr(p + 3, " - ");    // Tìm d?u g?ch ngang th? hai
    if (!p) return -1;
    
    p += 3; // Nh?y qua chu?i " - "
    int32_t val = 0;
    while (*p >= '0' && *p <= '9') { 
        val = val * 10 + (*p - '0'); 
        p++; 
    }
    return val;
}

int main(void)
{
    GPIO_Config_TX_RX();   
    USART1_Config();       // Debug PC
    USART2_Config();       // Giao tiep ESP32
    Timer2_Init();
    DHT11_Init();          

    PC_Println("STM32 Node 2 - READY");
    PC_Println("Sinh vien: Linh - DT177");

    Message_t tx_msg, rx_msg;
    char payload[80];
    char s_temp[8], s_hum[8];

    while (1)
    {
        //  Ð?I NH?N B?N TIN T? ESP32 (NODE 1)
        if (MSG_Receive_ESP32(&rx_msg)) 
        {
            rx_msg.data[rx_msg.len] = 0; // K?t thúc chu?i

            // N?u nh?n du?c l?nh ÐI?U KHI?N (CMD_ACK)
            if (rx_msg.cmd == CMD_ACK) {
                if (strcmp((char*)rx_msg.data, "LED_ON") == 0) {
                    GPIO_ResetBits(GPIOC, GPIO_Pin_13); // B?T LED
                    PC_Println(">> [NH?N L?NH ESP32] -> Ðã B?T ÐÈN LED (PC13)!");
                } 
                else if (strcmp((char*)rx_msg.data, "LED_OFF") == 0) {
                    GPIO_SetBits(GPIOC, GPIO_Pin_13);   // T?T LED
                    PC_Println(">> [NH?N L?NH ESP32] -> Ðã T?T ÐÈN LED (PC13)!");
                }
            }
            // N?u nh?n du?c b?n tin BÌNH THU?NG (CMD_TEXT)
            else if (rx_msg.cmd == CMD_TEXT) {
                PC_Print("[RX<-ESP32] Nh?n Lux: ");
                PC_Println((char*)rx_msg.data);

                //  Ð?C C?M BI?N DHT11
                DHT11_Read();

                // G?I B?N TIN PH?N H?I SANG ESP32 
                i16_to_str(s_temp, g_temp);
                i16_to_str(s_hum,  g_hum);

                strcpy(payload, "Phan hoi tu Node 2 ve Node 1: T:");
                strcat(payload, s_temp);
                strcat(payload, "C H:");
                strcat(payload, s_hum);
                strcat(payload, "%");

                MSG_Build(&tx_msg, CMD_TEXT, (uint8_t*)payload, (uint8_t)strlen(payload));
                MSG_Send_ESP32(&tx_msg);
                PC_Print("[TX->ESP32] "); PC_Println(payload);
            }
        }
    }
}