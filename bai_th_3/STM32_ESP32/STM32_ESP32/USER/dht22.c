/*
 * dht22.c
 * DHT22 - Data pin PA1
 * STM32F103C8T6
 */

#include "dht22.h"

// ---- Macro dieu khien PA1 ----
#define DHT_PIN_OUTPUT() \
    do { \
        GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1); \
        GPIOA->CRL |= GPIO_CRL_MODE1_1; /* Output 2MHz push-pull */ \
    } while(0)

#define DHT_PIN_INPUT() \
    do { \
        GPIOA->CRL &= ~(GPIO_CRL_MODE1 | GPIO_CRL_CNF1); \
        GPIOA->CRL |= GPIO_CRL_CNF1_1; /* Input pull-up/down */ \
        GPIOA->ODR |= (1 << 1);        /* Pull-up */ \
    } while(0)

#define DHT_HIGH()   (GPIOA->BSRR = GPIO_BSRR_BS1)
#define DHT_LOW()    (GPIOA->BSRR = GPIO_BSRR_BR1)
#define DHT_READ()   ((GPIOA->IDR >> 1) & 0x01)

// Delay micro-giay don gian (khong chinh xac tuyet doi nhung du dung)
static void delay_us(uint32_t us)
{
    // 72MHz: 1us ~ 72 chu ky, vong lap ~4 chu ky
    volatile uint32_t count = us * 18;
    while (count--);
}

void DHT22_Init(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_IOPAEN;
    DHT_PIN_OUTPUT();
    DHT_HIGH();
}

static uint8_t DHT22_ReadByte(void)
{
    uint8_t byte = 0;
    for (int i = 7; i >= 0; i--)
    {
        // Cho tin hieu LOW ket thuc (bat dau bit)
        while (DHT_READ() == 0);
        delay_us(35); // Doc sau 35us: neu van HIGH => bit 1, neu LOW => bit 0
        if (DHT_READ())
            byte |= (1 << i);
        while (DHT_READ() == 1); // Cho het bit
    }
    return byte;
}

DHT22_Data_t DHT22_Read(void)
{
    DHT22_Data_t result = {0, 0, 0};
    uint8_t data[5] = {0};

    // --- Gui tin hieu START ---
    DHT_PIN_OUTPUT();
    DHT_LOW();
    Delay_ms(2); // Keo LOW toi thieu 1ms, dung 2ms
    DHT_HIGH();
    delay_us(30);

    // --- Chuyen sang INPUT cho ---
    DHT_PIN_INPUT();

    // --- Cho DHT phan hoi: LOW 80us + HIGH 80us ---
    uint32_t timeout = 10000;
    while (DHT_READ() == 1 && timeout--);
    if (timeout == 0) return result;

    timeout = 10000;
    while (DHT_READ() == 0 && timeout--);
    if (timeout == 0) return result;

    timeout = 10000;
    while (DHT_READ() == 1 && timeout--);
    if (timeout == 0) return result;

    // --- Doc 40 bit du lieu ---
    for (int i = 0; i < 5; i++)
        data[i] = DHT22_ReadByte();

    // --- Kiem tra checksum ---
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) return result;

    // --- Giai ma ---
    uint16_t raw_hum  = ((uint16_t)data[0] << 8) | data[1];
    uint16_t raw_temp = ((uint16_t)(data[2] & 0x7F) << 8) | data[3];

    result.humidity    = raw_hum  / 10.0f;
    result.temperature = raw_temp / 10.0f;
    if (data[2] & 0x80) result.temperature = -result.temperature;
    result.valid = 1;

    return result;
}