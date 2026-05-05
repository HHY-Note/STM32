#include "stm32f10x.h"
#include "OLED.h"
#include "delay.h"
#include "sys.h"
#include "usart.h" 
#include "onenet.h"
#include "esp8266.h"
#include "Timer.h"
#include "ADC.h"
#include "GPIO.h"
#include "ds18b20.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

const char devPubTopic[] = "$sys/7JRHZ9cq38/test/thing/property/post";
const char *devSubTopic[] = {"$sys/7JRHZ9cq38/test/thing/property/set"};
unsigned char *dataPtr = NULL;
char PUBLIS_BUF[256];

int Ph_L=0,Ph_H=140,Tem_L=-100,Tem_H=100,Lig_L=0,Lig_H=100;
bool Warning;

uint16_t Temp;
double Ph_r;
uint16_t Ph;
double Lig_r;
uint16_t Lig;

uint16_t Count=0;
uint8_t Error_Count = 0;

void JsonValue()
{
	memset(PUBLIS_BUF, 0, sizeof(PUBLIS_BUF));
	sprintf(PUBLIS_BUF,"{\"id\":\"123\",\"params\":{\"Temp\":{\"value\":%d},\"Ph\":{\"value\":%d},\"Light\":{\"value\":%d}}}", Temp, Ph, Lig);
}

void Network_Init_Routine(void)
{
	OLED_Clear();
	OLED_ShowString(1,1,"8266 connect...");
	ESP8266_Init();	
	OLED_Clear();
	OLED_ShowString(1,1,"Conn OneNet...");
	while(OneNet_DevLink())
	{
		delay_ms(50);
	}
	OLED_Clear();
	OLED_ShowString(1,1,"OneNet Sub...");
	OneNet_Subscribe(devSubTopic,1);
	
	OLED_Clear();
	OLED_ShowString(1,1,"Ready...");
	Error_Count = 0;
}

int main(void){
	
	OLED_Init();
	delay_init();
	TIM_IRQinit();
	AD_Init();
	GPIO_init();
	Usart_Init();
	OLED_ShowString(1,1,"Ds18b20 Init...");
	while(DS18B20_Init())
	{OLED_ShowString(1,1,"Ds18b20 Error");}
	delay_ms(1000);
	
	Network_Init_Routine();
	
	while(1){
		
		Temp = (int)DS18B20_Get_Temp()/10;
		OLED_ShowString(2,1,"Temp:");
		OLED_ShowNum(2,7,Temp,3);
		Ph_r = (AD_Value[0]/4096.0)*3.3;
		Ph_r = -5.7541*Ph_r+16.654;
		if(Ph_r>14.0)Ph_r=14.0;
		if(Ph_r<0)Ph_r=0;
		Ph = (int)(Ph_r*10);
		OLED_ShowString(3,1," Ph :");
		OLED_ShowNum(3,6,Ph/10,2);
		OLED_ShowString(3,8,".");
		OLED_ShowNum(3,9,Ph%10,1);
		Lig_r = AD_Value[1]*((3.3/2.3)/40.95);
		Lig = (int)Lig_r;
		OLED_ShowString(4,1,"Lig :");
		OLED_ShowNum(4,7,Lig,3);
		
		if(Ph<Ph_L||Ph>Ph_H||Temp<Tem_L||Temp>Tem_H||Lig<Lig_L||Lig>Lig_H)
			{Bee_ON();}
		else
			{Bee_OFF();}
			
		if(Count == 1){
			JsonValue();
			if(OneNet_Publish(devPubTopic, PUBLIS_BUF) != 0) 
			{
				Error_Count++;
				UsartPrintf(USART_DEBUG, "Send Error! Count: %d\r\n", Error_Count);
			}
			else
			{
				Error_Count = 0;
			}
			ESP8266_Clear();
			Count=0;
			if(Error_Count >= 1)
			{
				OLED_Clear();
				OLED_ShowString(1,1,"Connect Lost!");
				OLED_ShowString(2,1,"Reconnecting...");
				delay_ms(1000);
				Network_Init_Routine(); 
			}
		}
		
		dataPtr = ESP8266_GetIPD(2);
		if(dataPtr != NULL)
			OneNet_RevPro(dataPtr);
		
	}
}
