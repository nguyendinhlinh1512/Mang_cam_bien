#include "define.h"

static void PrintUID(uint8_t *uid)
{
    uint8_t i;
    char    hex[3];
    char    nibble;

    for (i = 0; i < 4; i++)
    {
        nibble = (uid[i] >> 4) & 0x0F;
        hex[0] = nibble < 10 ? nibble + '0' : nibble - 10 + 'A';

        nibble = uid[i] & 0x0F;
        hex[1] = nibble < 10 ? nibble + '0' : nibble - 10 + 'A';

        hex[2] = '\0';
        Uart_SendStr(hex);

        if (i < 3)
            Uart_SendChar(':');
    }
    Uart_SendStr("\r\n");
}


int main(void)
{
    uint8_t uid[5];
    uint8_t lastUID[4];
    uint8_t blockData[16];
    uint8_t sak;
    uint8_t status;
    uint8_t i;
    uint8_t isNewCard;

    uint8_t defaultKey[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

    for (i = 0; i < 4; i++)
        lastUID[i] = 0;

    SystemInit();
    SysTick_Init();
    Uart_Gpio_TxRx_Init();
    Uart_Init();
    RC522_Init();

    Uart_SendStr(" RFID RC522 DEMO \r\n");
    Uart_SendStr("Scan RFID\r\n");
    Uart_SendStr("     ---------------------\r\n");

    while (1)
    {
        if (RC522_IsCardPresent(uid) == RC522_OK)
        {
            /* Kiem tra the moi */
            isNewCard = (uid[0] != lastUID[0] || uid[1] != lastUID[1] ||
                         uid[2] != lastUID[2] || uid[3] != lastUID[3]);

            if (isNewCard)
            {
                for (i = 0; i < 4; i++)
                    lastUID[i] = uid[i];

                Uart_SendStr("The phat hien!\r\n");
                Uart_SendStr("UID: ");
                PrintUID(uid);

                /* --- Buoc 1: Select the, lay SAK --- */
                sak = RC522_SelectTag(uid);

                if (sak == 0)
                {
                    /* [FIX] Them kiem tra SAK - neu = 0 la SelectTag that bai */
                    Uart_SendStr("Select Tag fail! Kiem tra SPI.\r\n");
                    RC522_Halt();
                    Uart_SendStr("     ---------------------\r\n");
                    Delay_ms(500);
                    continue;
                }

                Uart_SendStr("SAK: 0x");
                Uart_SendChar(((sak >> 4) < 10) ? (sak >> 4) + '0' : (sak >> 4) - 10 + 'A');
                Uart_SendChar(((sak & 0x0F) < 10) ? (sak & 0x0F) + '0' : (sak & 0x0F) - 10 + 'A');
                Uart_SendStr("\r\n");

                /* --- Buoc 2: Xac thuc sector --- */
                status = RC522_Auth(PICC_AUTHENT_1A, 4, defaultKey, uid);

                if (status == RC522_OK)
                {
                    Uart_SendStr("Xac thuc OK!\r\n");

                    /* --- Buoc 3: Doc block 4 --- */
                    if (RC522_ReadBlock(4, blockData) == RC522_OK)
                    {
                        Uart_SendStr("Block 4: ");
                        for (i = 0; i < 16; i++)
                        {
                            char nibble;
                            nibble = (blockData[i] >> 4) & 0x0F;
                            Uart_SendChar(nibble < 10 ? nibble + '0' : nibble - 10 + 'A');
                            nibble = blockData[i] & 0x0F;
                            Uart_SendChar(nibble < 10 ? nibble + '0' : nibble - 10 + 'A');
                            Uart_SendChar(' ');
                        }
                        Uart_SendStr("\r\n");
                    }
                    else
                    {
                        Uart_SendStr("Doc block fail!\r\n");
                    }
                }
                else
                {
                    Uart_SendStr("Xac thuc fail! (Sai key hoac the khong ho tro)\r\n");
                }

                RC522_Halt();
                Uart_SendStr("     ---------------------\r\n");
                Delay_ms(500);
            }
        }
        else
        {
            /* Khong co the: reset lastUID */
            for (i = 0; i < 4; i++)
                lastUID[i] = 0;

            Delay_ms(50);
        }
    }
}
