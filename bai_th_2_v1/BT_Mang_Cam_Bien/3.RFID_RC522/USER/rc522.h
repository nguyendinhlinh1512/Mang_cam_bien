/**
 * @file    rc522.h
 * @brief   Thu vien RFID RC522 su dung giao thuc SPI
 *          Ho tro doc UID the Mifare 1K / Mifare Mini / Ultralight
 *          Xac thuc va doc/ghi block du lieu
 *
 * Phu thuoc: spi.h / spi.c
 *
 * Ket noi phan cung:
 *   RC522 Pin  ->  STM32F103
 *   SDA (CS)   ->  PA4
 *   SCK        ->  PA5
 *   MOSI       ->  PA7
 *   MISO       ->  PA6
 *   RST        ->  PB0
 *   3.3V       ->  3.3V
 *   GND        ->  GND
 */

#ifndef RC522_H
#define RC522_H

#include "define.h"

//===========================//
//    CHAN RESET              //
//===========================//

/** Chan RST cua RC522 */
#define RC522_RST_PORT    GPIOB
#define RC522_RST_PIN     GPIO_Pin_0

//===========================//
//   DIA CHI THANH GHI       //
//===========================//

#define REG_COMMAND         0x01  /* Lenh dang thuc hien                */
#define REG_COM_I_EN        0x02  /* Cho phep ngat truyen thong         */
#define REG_DIV_I_EN        0x03  /* Cho phep ngat                      */
#define REG_COM_IRQ         0x04  /* Bit co ngat truyen thong           */
#define REG_DIV_IRQ         0x05  /* Bit co ngat                        */
#define REG_ERROR           0x06  /* Bit loi tu byte du lieu truoc      */
#define REG_STATUS1         0x07  /* Thong tin trang thai truyen thong  */
#define REG_STATUS2         0x08  /* Thong tin thu phat va bao mat      */
#define REG_FIFO_DATA       0x09  /* Vao / ra FIFO buffer 64 byte       */
#define REG_FIFO_LEVEL      0x0A  /* So byte duoc luu trong FIFO        */
#define REG_WATER_LEVEL     0x0B  /* Muc FIFO de bao canh bao tran      */
#define REG_CONTROL         0x0C  /* Dieu khien ngoai vi khac nhau      */
#define REG_BIT_FRAMING     0x0D  /* Dieu chinh truyen bit              */
#define REG_COLL            0x0E  /* Vi tri xung dot bit trong bus      */
#define REG_MODE            0x11  /* Xac dinh cai dat chung             */
#define REG_TX_MODE         0x12  /* Xac dinh toc do truyen du lieu     */
#define REG_RX_MODE         0x13  /* Xac dinh toc do nhan du lieu       */
#define REG_TX_CONTROL      0x14  /* Dieu khien chan logic anten TX1/TX2*/
#define REG_TX_ASK          0x15  /* Dieu khien cai dat dieu che ASK    */
#define REG_CRC_RESULT_H    0x21  /* Byte cao ket qua tinh CRC          */
#define REG_CRC_RESULT_L    0x22  /* Byte thap ket qua tinh CRC         */
#define REG_T_MODE          0x2A  /* Cai dat Timer                      */
#define REG_T_PRESCALER     0x2B  /* Chia tan so Timer                  */
#define REG_T_RELOAD_H      0x2C  /* Byte cao gia tri tai nap Timer     */
#define REG_T_RELOAD_L      0x2D  /* Byte thap gia tri tai nap Timer    */

//===========================//
//   LENH PCD (RC522)        //
//===========================//

#define PCD_IDLE            0x00  /* Huy lenh dang chay                 */
#define PCD_CALC_CRC        0x03  /* Kich hoat bo tinh CRC              */
#define PCD_TRANSMIT        0x04  /* Truyen du lieu tu FIFO             */
#define PCD_TRANSCEIVE      0x0C  /* Truyen tu FIFO va nhan vao FIFO    */
#define PCD_MF_AUTHENT      0x0E  /* Xac thuc Mifare                    */
#define PCD_SOFT_RESET      0x0F  /* Reset phan mem                     */

//===========================//
//   LENH PICC (THE)         //
//===========================//

#define PICC_REQIDL         0x26  /* Tim the dang idle                  */
#define PICC_REQALL         0x52  /* Tim tat ca the                     */
#define PICC_ANTICOLL       0x93  /* Chong xung dot cascade level 1     */
#define PICC_SELECT_TAG     0x93  /* Chon the                           */
#define PICC_AUTHENT_1A     0x60  /* Xac thuc voi key A                 */
#define PICC_AUTHENT_1B     0x61  /* Xac thuc voi key B                 */
#define PICC_READ           0x30  /* Doc 16 byte tu 1 block             */
#define PICC_WRITE          0xA0  /* Ghi 16 byte vao 1 block            */
#define PICC_HALT           0x50  /* Dung the                           */

//===========================//
//      MA TRANG THAI        //
//===========================//

#define RC522_OK            0     /* Thanh cong                         */
#define RC522_NOTAGERR      1     /* Khong tim thay the                 */
#define RC522_ERR           2     /* Loi truyen thong                   */

//===========================//
//       HANG SO             //
//===========================//

/** Do dai toi da du lieu nhan ve (byte) */
#define RC522_MAX_LEN       16

//===========================//
//          API              //
//===========================//

/**
 * @brief  Khoi dong RC522: cau hinh RST GPIO, reset chip,
 *         cai dat timer, modulation va bat anten
 *         Goi SPI1_Config() truoc hoac trong ham nay
 */
void RC522_Init(void);

/**
 * @brief  Bat anten phat song RF
 */
void RC522_AntennaOn(void);

/**
 * @brief  Tat anten phat song RF
 */
void RC522_AntennaOff(void);

/**
 * @brief  Phat hien the RFID trong vung doc
 * @param  reqMode   Kieu tim kiem: PICC_REQIDL hoac PICC_REQALL
 * @param  tagType   [out] 2 byte ATQA tra ve tu the
 * @return RC522_OK neu co the, RC522_NOTAGERR / RC522_ERR neu khong
 */
uint8_t RC522_Request(uint8_t reqMode, uint8_t *tagType);

/**
 * @brief  Chong xung dot, lay UID 4 byte cua the
 * @param  serNum    [out] 4 byte UID + 1 byte BCC (tong 5 byte)
 * @return RC522_OK neu thanh cong
 */
uint8_t RC522_Anticoll(uint8_t *serNum);

/**
 * @brief  Chon the theo UID, tra ve SAK (Select Acknowledge)
 * @param  serNum    4 byte UID + 1 byte BCC
 * @return SAK (1 byte) neu thanh cong, 0 neu loi
 */
uint8_t RC522_SelectTag(uint8_t *serNum);

/**
 * @brief  Xac thuc block truoc khi doc / ghi
 * @param  authMode   PICC_AUTHENT_1A (Key A) hoac PICC_AUTHENT_1B (Key B)
 * @param  blockAddr  Dia chi block can xac thuc
 * @param  sectorKey  Con tro toi 6 byte khoa
 * @param  serNum     4 byte UID cua the
 * @return RC522_OK neu xac thuc thanh cong
 */
uint8_t RC522_Auth(uint8_t authMode, uint8_t blockAddr,
                   uint8_t *sectorKey, uint8_t *serNum);

/**
 * @brief  Doc 16 byte du lieu tu 1 block (yeu cau xac thuc truoc)
 * @param  blockAddr  Dia chi block can doc (0..63 voi Mifare 1K)
 * @param  recvData   [out] Buffer nhan 16 byte du lieu
 * @return RC522_OK neu doc thanh cong
 */
uint8_t RC522_ReadBlock(uint8_t blockAddr, uint8_t *recvData);

/**
 * @brief  Ghi 16 byte du lieu vao 1 block (yeu cau xac thuc truoc)
 * @param  blockAddr   Dia chi block can ghi
 * @param  writeData   Con tro toi 16 byte du lieu can ghi
 * @return RC522_OK neu ghi thanh cong
 */
uint8_t RC522_WriteBlock(uint8_t blockAddr, uint8_t *writeData);

/**
 * @brief  Ket thuc phien lam viec, dung the (HALT + tat Crypto)
 */
void RC522_Halt(void);

/**
 * @brief  Ket hop Request + Anticoll: kiem tra co the va lay UID
 * @param  uid   [out] 4 byte UID (index 0..3), byte thu 4 la BCC
 * @return RC522_OK neu co the san sang, RC522_ERR neu khong co the
 */
uint8_t RC522_IsCardPresent(uint8_t *uid);

#endif /* RC522_H */