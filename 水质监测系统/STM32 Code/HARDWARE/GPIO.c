#include "stm32f10x.h"                  // Device header
void GPIO_init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);		//开启GPIOA的时钟
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);						//将PA1和PA2引脚初始化为推挽输出
	
	GPIO_SetBits(GPIOA, GPIO_Pin_11 | GPIO_Pin_12);				//设置PA1和PA2引脚为高电平
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOC, GPIO_Pin_13);	
}


void Bee_ON(void)
{
	GPIO_ResetBits(GPIOA, GPIO_Pin_11);
	GPIO_ResetBits(GPIOA, GPIO_Pin_12);
	GPIO_ResetBits(GPIOC, GPIO_Pin_13);
}


void Bee_OFF(void)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_11);
	GPIO_SetBits(GPIOA, GPIO_Pin_12);
	GPIO_SetBits(GPIOC, GPIO_Pin_13);
}

void Bee_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_11) == 0)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_11);
		GPIO_SetBits(GPIOA, GPIO_Pin_12);
	}
	else
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_11);
		GPIO_ResetBits(GPIOA, GPIO_Pin_12);
	}
}