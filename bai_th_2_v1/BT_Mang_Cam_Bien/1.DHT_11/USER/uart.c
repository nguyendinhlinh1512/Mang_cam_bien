#include "uart.h"

char 	data; 					 
int 	count=0; 		
char	mystr[100];
extern uint8_t flag;

void Uart_Init()
{
	USART_InitTypeDef usart_typedef;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  usart_typedef.USART_BaudRate = 9600;
	usart_typedef.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	usart_typedef.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	usart_typedef.USART_Parity = USART_Parity_No;
	usart_typedef.USART_StopBits = USART_StopBits_1;
	usart_typedef.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &usart_typedef);
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);//Cho phép ng?t khi b? d?m nh?n d?y
	NVIC_EnableIRQ(USART1_IRQn); // dki hàm ngat
	USART_Cmd(USART1, ENABLE);
}

void Uart_Gpio_TxRx_Init()
{
	GPIO_InitTypeDef gpio_typedef;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
// congifgure pin Tx - A9;
  gpio_typedef.GPIO_Pin = GPIO_Pin_9;
  gpio_typedef.GPIO_Mode = GPIO_Mode_AF_PP; //Ðây là ch? d? b?t bu?c d? chân GPIO có th? ch?u s? di?u khi?n tr?c ti?p t? b? ngo?i vi USART bên trong chip.
  gpio_typedef.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA,&gpio_typedef);
// configure pin Rx - A10;
  gpio_typedef.GPIO_Pin = GPIO_Pin_10;
  gpio_typedef.GPIO_Mode = GPIO_Mode_IN_FLOATING; //d? chân ? tr?ng thái lo l?ng, s?n sàng nh?n tín hi?u di?n áp t? module USB-to-TTL.
  gpio_typedef.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA,&gpio_typedef);
}


void Uart_SendChar(char _chr)
{
  USART_SendData(USART1, _chr);
  while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET); //
}

void Uart_SendStr(char *str)
{
 while(*str)
 {
	USART_SendData(USART1,*str);
	while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
	str++;
 }
}

void Uart_SendInt(int number)
{
 char count = 0;
 char digit[7] = "";
 if(number==0)
 {
	 USART_SendData(USART1,'0');
	 while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
 }
 if(number>0)
 {
	 while(number!=0)
	 {
		 digit[count]=number%10;
		 count++;
		 number=number/10;
	 }
	 while(count!=0)
	 {
		 USART_SendData(USART1,digit[count-1]+0x30);
		 while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
		 count--;
	 }
 }
 if(number<0)
 {
	 number=-number;
	 while(number!=0)
	 {
		 digit[count]=number%10;
		 count++;
		 number=number/10;
	 }
	 USART_SendData(USART1,'-');
	 while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
	 while(count!=0)
	 {
		 USART_SendData(USART1,digit[count-1]+0x30);
		 while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
		 count--;
	 }
 }
}

//void Uart_SendFloat(float number)
//{
//	uint8_t count1=0;
//	uint8_t count2=0;
//	char arr1[10]="";
//	char arr2[10]="";
//	int x,z;
//	float y;
//	
//	if(number==0.0)
//	{
//		USART_SendData(USART1,'0');
//		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
//	}
//	
//	if(number>0)
//	{
//		x= (int)number;
//		y= (float)number - x;
//		z=  y*1000;
//		
//		while (x != 0) 
//    {
//      arr1[count1] = x%10;
//      count1++;
//      x = x/10;
//    }
//		
//		while (count1!=0)
//		{
//			USART_SendData(USART1,arr1[count1-1]+0x30);
//			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
//			count1--;
//		}
//		
//		USART_SendData(USART1,'.');
//		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
//		
//		while (z != 0) 
//    {
//      arr2[count2] = z%10;
//      count2++;
//      z = z/10;
//    }
//		
//		while(count2 !=0)
//		{
//			USART_SendData(USART1,arr2[count2-1]+0x30);
//			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
//			count2--;
//		}
//	}
//	
//	if(number<0)
//	{
//		number=number*(-1);
//		x= (int)number;
//		y= (float)number - x;
//		z= y*1000;
//		
//		while (x != 0) 
//    {
//      arr1[count1] = x%10;
//      count1++;
//      x = x/10;
//    }
//		
//		USART_SendData(USART1,'-');
//		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
//		
//		while (count1!=0)
//		{
//			USART_SendData(USART1,arr1[count1-1]+0x30);
//			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
//			count1--;
//		}
//		
//		USART_SendData(USART1,'.');
//		while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
//		
//		while (z != 0) 
//    {
//      arr2[count2] = z%10;
//      count2++;
//      z = z/10;
//    }
//		
//		while(count2 !=0)
//		{
//			USART_SendData(USART1,arr2[count2-1]+0x30);
//			while(USART_GetFlagStatus(USART1,USART_FLAG_TXE)==RESET);
//			count2--;
//		}
//	}
//}
void Uart_SendFloat(float number)
{
    if (number < 0) {
        Uart_SendChar('-');
        number = -number;
    }

    int integerPart = (int)number;
    Uart_SendInt(integerPart); 

    Uart_SendChar('.');

    int fractionPart = (int)((number - (float)integerPart) * 100.0f + 0.5f);

    // vd .05

    if (fractionPart < 10) {
        Uart_SendChar('0'); 
    }
    
    //  00
    Uart_SendInt(fractionPart);
}

uint16_t Uart_ReadChar()
{
	return USART_ReceiveData(USART1);
}

//uint8_t Uart_Compare(char *str1, char * str2)
//{
//	while(*str1 && *str2)
//	{
//		if(*str1 != *str2)
//		{
//			return 0;
//		}
//		str1++;
//		str2++;
//	}
//	if (*str1 == '\0' && *str2 == '\0') 
//	{
//   return 1; 
//  } 
//	else 
//	{
//   return 0; 
//  }
//}

void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        data = Uart_ReadChar();

        if (data == '\r' || data == '\n')
        {
            if (count > 0)
            {
                flag = 1;
                mystr[count] = '\0';
                count = 0;
            }
        }
        else
        {
            if (count < 99)
            {
                mystr[count] = data;
                count++;
            }
        }
    }
}


uint8_t Uart_Debug_Flag()
{
	if(flag==1)
	{
		flag=0;
		return 1;
	}
	return 0;
}
