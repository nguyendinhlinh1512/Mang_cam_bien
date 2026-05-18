#include "define.h"

int main() {
	Uart_Gpio_TxRx_Init();
	DHT11_Init();
	Led_Debug();
	Timer4_Init();
	Uart_Init();
	off();
	
	while(1){
    DHT11_Read();
    
    Uart_SendStr("Temperature: ");
    // S?a t? Uart_SendInt sang Uart_SendFloat
    Uart_SendFloat(DHT11_Get_Temperature()); 
    Uart_SendStr(" C\n");
    
    Uart_SendStr("Humidity: ");
    // S?a t? Uart_SendInt sang Uart_SendFloat
    Uart_SendFloat(DHT11_Get_Humidity()); 
    Uart_SendStr(" %\n");
    
    delay_ms(1000);
	}
}
