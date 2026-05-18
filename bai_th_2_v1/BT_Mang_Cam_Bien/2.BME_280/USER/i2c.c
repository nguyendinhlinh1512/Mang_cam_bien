/**
 * @file    i2c.c
 * @brief   Thu vien I2C1 cho STM32F10x (Standard Peripheral Library)
 *
 * Phan cung su dung:
 *   - I2C1: PB6 (SCL), PB7 (SDA)
 *   - Toc do: 100kHz (Standard Mode)
 *   - Che do chan: Alternate Function Open-Drain
 */

#include "i2c.h"

//===========================//
//        I2C CONFIG         //
//===========================//

void I2C1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef  I2C_InitStructure;

    /* Cap clock cho GPIOB va I2C1 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C1,  ENABLE);

    /* PB6: SCL, PB7: SDA - Alternate Function Open-Drain */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    /* Cau hinh I2C1 */
    I2C_InitStructure.I2C_ClockSpeed          = 100000;                  /* 100 kHz Standard Mode       */
    I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;            /* Che do I2C thuan             */
    I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2;         /* Duty cycle (cho Fast mode)   */
    I2C_InitStructure.I2C_OwnAddress1         = 0x00;                    /* Dia chi khi lam Slave (0=off) */
    I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;          /* Tu dong gui ACK              */
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit; /* Dia chi 7-bit           */

    I2C_Init(I2C1, &I2C_InitStructure);
    I2C_Cmd(I2C1, ENABLE);
}

//===========================//
//      GHI 1 BYTE           //
//===========================//

void I2C1_WriteReg(uint8_t addr, uint8_t reg, uint8_t data)
{
    /* Cho bus ranh */
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    // tao xung start de dieu khien bus
    I2C_GenerateSTART(I2C1, ENABLE);
	  // Ch? s? ki?n EV5 (Xác nh?n dã g?i START thành công).
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    /* Gui dia chi slave (ghi) */
    I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Transmitter);
	  // Ch? s? ki?n EV6 (Slave dã ph?n h?i ACK cho d?a ch?).
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    /* Gui dia chi thanh ghi mu?n ghi. */
    I2C_SendData(I2C1, reg);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    /* Gui du lieu,  */
    I2C_SendData(I2C1, data);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    /* STOP, T?o xung STOP d? k?t thúc phiên làm vi?c. */
    I2C_GenerateSTOP(I2C1, ENABLE);
}

//===========================//
//      DOC 1 BYTE           //
//===========================//

uint8_t I2C1_ReadReg(uint8_t addr, uint8_t reg)
{
    uint8_t value;

    /* Cho bus ranh */
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    /* START */
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    /* Gui dia chi slave (ghi) de gui dia chi thanh ghi */
    I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    /* Gui dia chi thanh ghi */
    I2C_SendData(I2C1, reg);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    /* REPEATED START */
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    /* Gui dia chi slave (doc) */
    I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    /* Tat ACK truoc khi nhan byte cuoi (chi 1 byte) de gui NACK */
    I2C_AcknowledgeConfig(I2C1, DISABLE);

    /* Cho nhan xong 1 byte */
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    value = I2C_ReceiveData(I2C1);

    /* STOP */
    I2C_GenerateSTOP(I2C1, ENABLE);

    /* Bat lai ACK cho lan doc tiep theo */
    I2C_AcknowledgeConfig(I2C1, ENABLE);

    return value;
}

//===========================//
//    DOC NHIEU BYTE         //
//===========================//

void I2C1_ReadMulti(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t len)
{
    int i;

    /* Cho bus ranh */
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    /* START */
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    /* Gui dia chi slave (ghi) */
    I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    /* Gui dia chi thanh ghi bat dau */
    I2C_SendData(I2C1, reg);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    /* REPEATED START */
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    /* Gui dia chi slave (doc) */
    I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    /* Nhan tung byte; tat ACK truoc byte cuoi de gui NACK */
    for (i = 0; i < len; i++)
    {
        if (i == len - 1)
            I2C_AcknowledgeConfig(I2C1, DISABLE); /* Byte cuoi -> NACK */

        while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
        data[i] = I2C_ReceiveData(I2C1);
    }

    /* STOP */
    I2C_GenerateSTOP(I2C1, ENABLE);

    /* Bat lai ACK */
    I2C_AcknowledgeConfig(I2C1, ENABLE);
}