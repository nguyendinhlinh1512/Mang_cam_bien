/*
 * [FIX] RC522_SelectTag: sua recvBits == 0x18 -> 0x08 (SAK = 1 byte = 8 bit)
 * [FIX] RC522_ReadBlock: sua check unLen 0x90 -> 144 (ro rang hon)
 */

#include "rc522.h"

//===========================//
//   HAM NOI BO              //
//===========================//

/**
 *   Set mot so bit trong thanh ghi (OR voi mask)
 */
static void _SetBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp = SPI1_ReadReg(reg);
    SPI1_WriteReg(reg, tmp | mask);
}

/**
 *  Xoa mot so bit trong thanh ghi (AND NOT voi mask)
 */
static void _ClearBitMask(uint8_t reg, uint8_t mask)
{
    uint8_t tmp = SPI1_ReadReg(reg);
    SPI1_WriteReg(reg, tmp & (~mask));
}

/**
 *   Tinh CRC bang co-processor cua RC522, 2 byte check loi sao
 */
static void _CalcCRC(uint8_t *pIn, uint8_t len, uint8_t *pOut)
{
    uint16_t i;

    _ClearBitMask(REG_DIV_IRQ, 0x04);
    _SetBitMask(REG_FIFO_LEVEL, 0x80);
    SPI1_WriteReg(REG_COMMAND, PCD_IDLE);

    for (i = 0; i < len; i++)
        SPI1_WriteReg(REG_FIFO_DATA, pIn[i]);

    SPI1_WriteReg(REG_COMMAND, PCD_CALC_CRC);

    i = 5000;
    while (--i) {
        if (SPI1_ReadReg(REG_DIV_IRQ) & 0x04)
            break;
    }

    pOut[0] = SPI1_ReadReg(REG_CRC_RESULT_L);
    pOut[1] = SPI1_ReadReg(REG_CRC_RESULT_H);
}

/**
 * Loi giao tiep: gui lenh va du lieu, nhan ket qua tu the
 */
static uint8_t _ToCard(uint8_t command,
                        uint8_t *sendData, uint8_t sendLen,
                        uint8_t *backData, uint16_t *backLen)
{
    uint8_t  status   = RC522_ERR;
    uint8_t  irqEn    = 0x00;
    uint8_t  waitIRq  = 0x00;
    uint8_t  lastBits = 0;
    uint8_t  n        = 0;
    uint16_t i        = 0;

    if (command == PCD_MF_AUTHENT) {
        irqEn   = 0x12;
        waitIRq = 0x10;
    } else if (command == PCD_TRANSCEIVE) {
        irqEn   = 0x77;
        waitIRq = 0x30;
    }

    SPI1_WriteReg(REG_COM_I_EN,   irqEn | 0x80);
    _ClearBitMask(REG_COM_IRQ,    0x80);
    _SetBitMask(REG_FIFO_LEVEL,   0x80);
    SPI1_WriteReg(REG_COMMAND,    PCD_IDLE);

    for (i = 0; i < sendLen; i++)
        SPI1_WriteReg(REG_FIFO_DATA, sendData[i]);

    SPI1_WriteReg(REG_COMMAND, command);
    if (command == PCD_TRANSCEIVE)
        _SetBitMask(REG_BIT_FRAMING, 0x80);

    /* Cho phan hoi tu the (timeout ~25ms) */
    i = 2000;
    do {
        n = SPI1_ReadReg(REG_COM_IRQ);
        i--;
    } while (i && !(n & 0x01) && !(n & waitIRq));

    _ClearBitMask(REG_BIT_FRAMING, 0x80);

    if (i == 0)
        return RC522_ERR;

    if (!(SPI1_ReadReg(REG_ERROR) & 0x1B)) {
        status = RC522_OK;

        if (n & irqEn & 0x01)
            status = RC522_NOTAGERR;

        if (command == PCD_TRANSCEIVE) {
            n        = SPI1_ReadReg(REG_FIFO_LEVEL);
            lastBits = SPI1_ReadReg(REG_CONTROL) & 0x07;
            *backLen = lastBits ? (n - 1) * 8 + lastBits : n * 8;

            if (n == 0) n = 1;
            if (n > RC522_MAX_LEN) n = RC522_MAX_LEN;

            for (i = 0; i < n; i++)
                backData[i] = SPI1_ReadReg(REG_FIFO_DATA);
        }
    } else {
        status = RC522_ERR;
    }

    return status;
}


//       RC522_Init          

void RC522_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = RC522_RST_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(RC522_RST_PORT, &GPIO_InitStructure);

    SPI1_Config();

    /* Hardware reset */
    GPIO_ResetBits(RC522_RST_PORT, RC522_RST_PIN);
    Delay_ms(10);
    GPIO_SetBits(RC522_RST_PORT, RC522_RST_PIN);
    Delay_ms(50);

    /* Software reset */
    SPI1_WriteReg(REG_COMMAND, PCD_SOFT_RESET);
    Delay_ms(50);

    /* Timer config */
    SPI1_WriteReg(REG_T_MODE,      0x8D);
    SPI1_WriteReg(REG_T_PRESCALER, 0x3E);
    SPI1_WriteReg(REG_T_RELOAD_H,  0x00);
    SPI1_WriteReg(REG_T_RELOAD_L,  0x1E);

    /* 100% ASK modulation */
    SPI1_WriteReg(REG_TX_ASK,  0x40);

    /* CRC preset 0x6363 theo ISO14443 */
    SPI1_WriteReg(REG_MODE,    0x3D);

    RC522_AntennaOn();
}


//   RC522_AntennaOn/Off   

void RC522_AntennaOn(void)
{
    uint8_t temp = SPI1_ReadReg(REG_TX_CONTROL);
    if (!(temp & 0x03))
        _SetBitMask(REG_TX_CONTROL, 0x03);
}

void RC522_AntennaOff(void)
{
    _ClearBitMask(REG_TX_CONTROL, 0x03);
}


//      RC522_Request       

uint8_t RC522_Request(uint8_t reqMode, uint8_t *tagType)
{
    uint8_t  status;
    uint16_t backBits;

    SPI1_WriteReg(REG_BIT_FRAMING, 0x07);

    tagType[0] = reqMode;
    status = _ToCard(PCD_TRANSCEIVE, tagType, 1, tagType, &backBits);

    if ((status != RC522_OK) || (backBits != 0x10))
        status = RC522_ERR;

    return status;
}

//      RC522_Anticoll   lay uid 

uint8_t RC522_Anticoll(uint8_t *serNum)
{
    uint8_t  status;
    uint8_t  serNumCheck = 0;
    uint16_t unLen;
    uint8_t  cmd[2];
    uint8_t  i;

    SPI1_WriteReg(REG_BIT_FRAMING, 0x00);

    cmd[0] = PICC_ANTICOLL;
    cmd[1] = 0x20;

    status = _ToCard(PCD_TRANSCEIVE, cmd, 2, serNum, &unLen);

    if (status == RC522_OK) {
        for (i = 0; i < 4; i++)
            serNumCheck ^= serNum[i];

        if (serNumCheck != serNum[4])
            status = RC522_ERR;
    }

    return status;
}

//     RC522_SelectTag      chon de giao tiep
 
uint8_t RC522_SelectTag(uint8_t *serNum)
{
    uint8_t  buf[9];
    uint8_t  status;
    uint16_t recvBits;
    uint8_t  i;

    buf[0] = PICC_SELECT_TAG;
    buf[1] = 0x70;

    for (i = 0; i < 5; i++)
        buf[2 + i] = serNum[i];

    _CalcCRC(buf, 7, &buf[7]);

    status = _ToCard(PCD_TRANSCEIVE, buf, 9, buf, &recvBits);

    /*
     * RC522 clone (VERSION=0x82) tra ve SAK(1B) + CRC(2B) = 24 bit
     * RC522 chinh hang chi tra ve SAK(1B) = 8 bit
     * Dung >= 8 de tuong thich ca hai loai chip
     */
    if ((status == RC522_OK) && (recvBits >= 8))
        return buf[0];   /* SAK */

    return 0;
}

//        RC522_Auth     xthuc blk

uint8_t RC522_Auth(uint8_t authMode, uint8_t blockAddr,
                   uint8_t *sectorKey, uint8_t *serNum)
{
    uint8_t  buf[12];
    uint16_t recvBits;
    uint8_t  i;

    buf[0] = authMode;
    buf[1] = blockAddr;

    for (i = 0; i < 6; i++)
        buf[2 + i] = sectorKey[i];

    for (i = 0; i < 4; i++)
        buf[8 + i] = serNum[i];

    return _ToCard(PCD_MF_AUTHENT, buf, 12, buf, &recvBits);
}

//     RC522_ReadBlock    

uint8_t RC522_ReadBlock(uint8_t blockAddr, uint8_t *recvData)
{
    uint8_t  buf[4];
    uint16_t unLen;
    uint8_t  status;

    buf[0] = PICC_READ;
    buf[1] = blockAddr;
    _CalcCRC(buf, 2, &buf[2]);

    status = _ToCard(PCD_TRANSCEIVE, buf, 4, recvData, &unLen);

    /*
     * [FIX] 16 byte data + 2 byte CRC = 18 byte = 144 bit = 0x90
     * Giu nguyen gia tri 0x90, them comment ro rang de tranh nham
     */
    if ((status != RC522_OK) || (unLen != 144))
        status = RC522_ERR;

    return status;
}

//    RC522_WriteBlock    

uint8_t RC522_WriteBlock(uint8_t blockAddr, uint8_t *writeData)
{
    uint8_t  buf[18];
    uint16_t recvBits;
    uint8_t  status;
    uint8_t  i;

    buf[0] = PICC_WRITE;
    buf[1] = blockAddr;
    _CalcCRC(buf, 2, &buf[2]);

    status = _ToCard(PCD_TRANSCEIVE, buf, 4, buf, &recvBits);

    if ((status != RC522_OK) || (recvBits != 4) || ((buf[0] & 0x0F) != 0x0A))
        return RC522_ERR;

    for (i = 0; i < 16; i++)
        buf[i] = writeData[i];

    _CalcCRC(buf, 16, &buf[16]);

    status = _ToCard(PCD_TRANSCEIVE, buf, 18, buf, &recvBits);

    if ((status != RC522_OK) || (recvBits != 4) || ((buf[0] & 0x0F) != 0x0A))
        return RC522_ERR;

    return RC522_OK;
}

//       RC522_Halt       

void RC522_Halt(void)
{
    uint8_t  buf[4];
    uint16_t unLen;

    buf[0] = PICC_HALT;
    buf[1] = 0;
    _CalcCRC(buf, 2, &buf[2]);

    _ToCard(PCD_TRANSCEIVE, buf, 4, buf, &unLen);

    _ClearBitMask(REG_STATUS2, 0x08);
}

//   RC522_IsCardPresent  

uint8_t RC522_IsCardPresent(uint8_t *uid)
{
    uint8_t tagType[2];

    if (RC522_Request(PICC_REQIDL, tagType) != RC522_OK)
        return RC522_ERR;

    return RC522_Anticoll(uid);
}
