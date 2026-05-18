#include "dht11.h"
#include "uart.h"

int16_t g_temp = 0;
int16_t g_hum  = 0;

static void DHT11_Set_Output(void) {
	// de de dkhien xung xuong thap hay cao
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; 
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void DHT11_Set_Input(void) {
	// de nhan tin hieu dht ve
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}


static void DHT_Delay_us(uint16_t us) {
    uint16_t start = TIM2->CNT; // dem tu 0 den 999 moi 1ms
    while (1) {
        uint16_t now = TIM2->CNT;
        uint16_t elapsed = (now >= start) ? (now - start) : (1000 - start + now);
        if (elapsed >= us) break;	
    }
}
// ham chong treo chip 
static uint8_t Wait_Pin_State(uint8_t state, uint16_t timeout_us) {
    uint16_t start = TIM2->CNT;
    while (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12) != state) {
        uint16_t now = TIM2->CNT;
        uint16_t elapsed = (now >= start) ? (now - start) : (1000 - start + now);
        if (elapsed >= timeout_us) return 0; // Het gio
    }
    return 1;
}

static uint8_t DHT11_ReadByte(void) {
    uint8_t byte = 0;
    for (int i = 0; i < 8; i++) {
        if (!Wait_Pin_State(1, 100)) return 0;
        
        // Dung TIM2 de delay chinh xac 35 micro-giay
        DHT_Delay_us(35); 
        
        byte <<= 1;
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12)) {
            byte |= 1; // Nhin thay muc CAO -> bit 1
            Wait_Pin_State(0, 100);
        }
    }
    return byte;
}

void DHT11_Init(void) {
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);
    
    GPIO_InitTypeDef gpioInit;
    gpioInit.GPIO_Mode  = GPIO_Mode_Out_PP;
    gpioInit.GPIO_Pin   = GPIO_Pin_13;
    gpioInit.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &gpioInit);

    DHT11_Set_Output();
    GPIO_SetBits(GPIOB, GPIO_Pin_12);
}

void DHT11_Read(void) {
    uint8_t u8Buff[5] = {0};
    
    DHT11_Set_Output();
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    Delay_ms(20);                       
    GPIO_SetBits(GPIOB, GPIO_Pin_12);   
    
    DHT11_Set_Input(); 
    __disable_irq(); // khoa moi ngat khac

    if (!Wait_Pin_State(0, 100)) { __enable_irq(); g_temp = -1; g_hum = -1; return; }
    if (!Wait_Pin_State(1, 100)) { __enable_irq(); g_temp = -2; g_hum = -2; return; }
    if (!Wait_Pin_State(0, 100)) { __enable_irq(); g_temp = -3; g_hum = -3; return; }

    for (int i = 0; i < 5; i++) {
        u8Buff[i] = DHT11_ReadByte();
    }

    __enable_irq(); 

    // Tinh Checksum
    if (((u8Buff[0] + u8Buff[1] + u8Buff[2] + u8Buff[3]) & 0xFF) == u8Buff[4] && u8Buff[4] != 0) {
        g_temp = u8Buff[2];
        g_hum  = u8Buff[0];
    } else {
        g_temp = -4; // Loi Checksum do nhiu
        g_hum  = -4;
    }
}