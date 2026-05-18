/**
 * @file    spi.c
 * @brief   Thu vien SPI1 cho STM32F10x (Standard Peripheral Library)
 *
 * Phan cung su dung:
 *   - SPI1: PA5 (SCK), PA6 (MISO), PA7 (MOSI), PA4 (CS)
 *   - Toc do: fPCLK/8 = 9MHz (SYSCLK = 72MHz)
 *   - Che do: SPI Mode 0 (CPOL=0, CPHA=0), MSB First, 8-bit
 */

#include "spi.h"

//===========================//
//        SPI1 CONFIG        //
//===========================//

void SPI1_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    /* Cap clock cho GPIOA va SPI1 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1,  ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,  ENABLE);

    /* PA4: CS - Output Push-Pull (dieu khien bang phan mem) */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* PA5: SCK, PA7: MOSI - Alternate Function Push-Pull */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* PA6: MISO - Input Floating */
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* CS idle = HIGH (giai phong bus) */
    SPI1_CS_High();

    /* Cau hinh SPI1 */
    SPI_InitStructure.SPI_Direction         = SPI_Direction_2Lines_FullDuplex; /* Full-duplex 2 duong         */
    SPI_InitStructure.SPI_Mode              = SPI_Mode_Master;                 /* Che do Master               */
    SPI_InitStructure.SPI_DataSize          = SPI_DataSize_8b;                 /* Truyen 8-bit moi lan        */
    SPI_InitStructure.SPI_CPOL              = SPI_CPOL_Low;                    /* CPOL=0: SCK idle = LOW      */
    SPI_InitStructure.SPI_CPHA              = SPI_CPHA_1Edge;                  /* CPHA=0: lay mau can 1       */
    SPI_InitStructure.SPI_NSS               = SPI_NSS_Soft;                    /* NSS dieu khien bang phan mem*/
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;         /* 72MHz / 8 = 9MHz            */
    SPI_InitStructure.SPI_FirstBit          = SPI_FirstBit_MSB;                /* MSB truyen truoc            */
    SPI_InitStructure.SPI_CRCPolynomial     = 7;                               /* CRC (khong su dung)         */

    SPI_Init(SPI1, &SPI_InitStructure);

    /* SSI = 1 khi dung NSS_Soft: tranh loi MODF */
    SPI_NSSInternalSoftwareConfig(SPI1, SPI_NSSInternalSoft_Set);

    SPI_Cmd(SPI1, ENABLE);
}

//===========================//
//    TRUYEN / NHAN 1 BYTE   //
//===========================//

uint8_t SPI1_TransmitReceive(uint8_t data)
{
    /* Cho Transmit buffer trong (TXE) */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);
    SPI_I2S_SendData(SPI1, data);

    /* Cho Receive buffer co du lieu (RXNE) */
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    return (uint8_t)SPI_I2S_ReceiveData(SPI1);
}

//===========================//
//       GHI 1 BYTE          //
//===========================//

void SPI1_WriteReg(uint8_t reg, uint8_t data)
{
    /* Byte dia chi: bit7=0 (write), bit[6:1]=reg, bit0=0 */
    uint8_t addr = (reg << 1) & 0x7E;

    SPI1_CS_Low();
    SPI1_TransmitReceive(addr);
    SPI1_TransmitReceive(data);
    SPI1_CS_High();
}

//===========================//
//       DOC 1 BYTE          //
//===========================//

uint8_t SPI1_ReadReg(uint8_t reg)
{
    uint8_t value;

    /* Byte dia chi: bit7=1 (read), bit[6:1]=reg, bit0=0 */
    uint8_t addr = ((reg << 1) & 0x7E) | 0x80;

    SPI1_CS_Low();
    SPI1_TransmitReceive(addr);
    value = SPI1_TransmitReceive(0x00);  /* Truyen dummy byte de nhan data */
    SPI1_CS_High();

    return value;
}
 