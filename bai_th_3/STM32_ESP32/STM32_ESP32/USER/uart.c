#include "uart.h"

// ============================================================
//  GPIO: USART1 PA9/PA10, USART2 PA2/PA3
// ============================================================
void GPIO_Config_TX_RX(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitTypeDef g;

    // USART1 TX = PA9 (AF Push-Pull)
    g.GPIO_Pin   = GPIO_Pin_9;
    g.GPIO_Mode  = GPIO_Mode_AF_PP;
    g.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &g);

    // USART1 RX = PA10 (Input Floating)
    g.GPIO_Pin  = GPIO_Pin_10;
    g.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &g);

    // USART2 TX = PA2 (AF Push-Pull)
    g.GPIO_Pin  = GPIO_Pin_2;
    g.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOA, &g);

    // USART2 RX = PA3 (Input Floating)
    g.GPIO_Pin  = GPIO_Pin_3;
    g.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &g);
}

// ============================================================
//  USART1 — 115200, giao tiep PC debug
// ============================================================
void USART1_Config(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);

    USART_InitTypeDef u;
    u.USART_BaudRate            = 115200;
    u.USART_WordLength          = USART_WordLength_8b;
    u.USART_StopBits            = USART_StopBits_1;
    u.USART_Parity              = USART_Parity_No;
    u.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    u.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART1, &u);

    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);

    NVIC_InitTypeDef n;
    n.NVIC_IRQChannel                   = USART1_IRQn;
    n.NVIC_IRQChannelPreemptionPriority = 1;
    n.NVIC_IRQChannelSubPriority        = 0;
    n.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&n);

    USART_Cmd(USART1, ENABLE);
}

// ============================================================
//  USART2 — 115200, giao tiep ESP32
// ============================================================
void USART2_Config(void)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);

    USART_InitTypeDef u;
    u.USART_BaudRate            = 115200;
    u.USART_WordLength          = USART_WordLength_8b;
    u.USART_StopBits            = USART_StopBits_1;
    u.USART_Parity              = USART_Parity_No;
    u.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    u.USART_Mode                = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &u);

    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    NVIC_InitTypeDef n;
    n.NVIC_IRQChannel                   = USART2_IRQn;
    n.NVIC_IRQChannelPreemptionPriority = 0;
    n.NVIC_IRQChannelSubPriority        = 0;
    n.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&n);

    USART_Cmd(USART2, ENABLE);
}

// ============================================================
//  USART1 — In chuoi ra PC
// ============================================================
void PC_Print(const char *str)
{
    while (*str) {
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, (uint8_t)*str++);
    }
}

void PC_Println(const char *str)
{
    PC_Print(str);
    PC_Print("\r\n");
}

// ============================================================
//  USART1 — In so nguyen (dung cho debug DHT11) 
// ============================================================
void USART1_Send_Number(int16_t num)
{
    char buf[8];
    uint8_t i = 0;

    if (num == 0) { PC_Print("0"); return; }
    if (num < 0)  { PC_Print("-"); num = -num; }

    char tmp[8];
    uint8_t n = 0;
    while (num > 0) { tmp[n++] = '0' + (num % 10); num /= 10; }
    for (int8_t k = n-1; k >= 0; k--) buf[i++] = tmp[k];
    buf[i] = 0;
    PC_Print(buf);
}

// ============================================================
//  USART1 IRQ — Nhan lenh tu PC (ket thuc bang Enter)
// ============================================================
static volatile char    pc_rx_buf[MAX_BUFFER];
static volatile uint8_t pc_rx_idx    = 0;
static volatile uint8_t pc_line_ready = 0;

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE))
    {
        char c = (char)USART_ReceiveData(USART1);
        // Echo
        while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
        USART_SendData(USART1, (uint8_t)c);

        if (c == '\r' || c == '\n') {
            if (pc_rx_idx > 0) {
                pc_rx_buf[pc_rx_idx] = '\0';
                pc_rx_idx    = 0;
                pc_line_ready = 1;
            }
        } else if (pc_rx_idx < MAX_BUFFER - 1) {
            pc_rx_buf[pc_rx_idx++] = c;
        }
    }
}

uint8_t PC_ReadLine(char *buf, uint8_t maxlen)
{
    if (!pc_line_ready) return 0;
    strncpy(buf, (const char*)pc_rx_buf, maxlen);
    pc_line_ready = 0;
    return 1;
}

// ============================================================
//  GIAO THUC BAN TIN
// ============================================================
uint8_t MSG_Checksum(Message_t *msg)
{
    uint8_t cs = msg->cmd ^ msg->len;
    for (int i = 0; i < msg->len; i++) cs ^= msg->data[i];
    return cs;
}

void MSG_Build(Message_t *msg, uint8_t cmd, uint8_t *data, uint8_t len)
{
    msg->start    = START_BYTE;
    msg->cmd      = cmd;
    msg->len      = len;
    memcpy(msg->data, data, len);
    msg->checksum = MSG_Checksum(msg);
    msg->end      = END_BYTE;
}

void MSG_Send_ESP32(Message_t *msg)
{
	// LENH CHO DOI TXE TRONG moi truyen
    #define TX2(b)  do { \
        while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET); \
        USART_SendData(USART2, (b)); \
    } while(0)

    TX2(msg->start);
    TX2(msg->cmd);
    TX2(msg->len);
    for (int i = 0; i < msg->len; i++) TX2(msg->data[i]);
    TX2(msg->checksum);
    TX2(msg->end);
}

// ============================================================
//  USART2 IRQ — Nhan ban tin tu ESP32
// ============================================================
static uint8_t          esp_rx_buf[MAX_BUFFER];
static uint8_t          esp_rx_idx   = 0;
static volatile uint8_t esp_msg_ready = 0;
static Message_t        esp_rx_msg;

void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE))
    {
        uint8_t byte = (uint8_t)USART_ReceiveData(USART2);

        // Bo qua byte rac truoc START_BYTE
        if (esp_rx_idx == 0 && byte != START_BYTE) return;

        esp_rx_buf[esp_rx_idx++] = byte;

        if (esp_rx_idx >= 3) {
            uint8_t total = 3 + esp_rx_buf[2] + 2;
            if (esp_rx_idx == total) {
                esp_rx_msg.start = esp_rx_buf[0];
                esp_rx_msg.cmd   = esp_rx_buf[1];
                esp_rx_msg.len   = esp_rx_buf[2];
                memcpy(esp_rx_msg.data, &esp_rx_buf[3], esp_rx_msg.len);
                esp_rx_msg.data[esp_rx_msg.len] = 0;
                esp_rx_msg.checksum = esp_rx_buf[3 + esp_rx_msg.len];
                esp_rx_msg.end      = esp_rx_buf[3 + esp_rx_msg.len + 1];
                esp_rx_idx = 0;

                if (esp_rx_msg.end      == END_BYTE &&
                    esp_rx_msg.checksum == MSG_Checksum(&esp_rx_msg))
                {
                    esp_msg_ready = 1;
                }
            }
        }

        if (esp_rx_idx >= MAX_BUFFER) esp_rx_idx = 0;
    }
}

uint8_t MSG_Receive_ESP32(Message_t *out_msg)
{
    if (!esp_msg_ready) return 0;
    *out_msg      = esp_rx_msg;
    esp_msg_ready = 0;
    return 1;
}