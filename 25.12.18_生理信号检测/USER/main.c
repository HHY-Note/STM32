#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "Timer.h"
#include "ADC.h"
#include "LED.h"

int Val[128]={0};      //记录原始ADC值
int val[128]={0};      //滤波后的ADC值
uint16_t Count=0;      //现在没用到
uint16_t count=0;      //定时器中断触发次数
double temp = 1;       //解决输出范围不匹配问题
uint8_t biaoji = 0;    //标记，检测跳动使用

uint8_t in_peak = 0;        //上升
uint8_t rise_count = 0;     //递增次数
#define MIN_RISE_SAMPLES 4  //最小上升次数

float current_time = 0;     //现在时间
float last_peak_time = 0;   //上次跳动时间
float bpm_instant = 0;      //心率


int main(void)
{
		LED_Init();
		OLED_Init();
		AD_Init();
		TIM_IRQinit();
		while (1);
}

//计算比较值
uint16_t Compare_Number(void)
{
    int sum = 0;
    for(int i=0;i<128;i++)
        sum += val[(count-i) & 0x7F];
    return sum / 128;
}


void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
	{
		count++;
		current_time += 0.03; //触发一次，时间增加0.03s
		
		Val[count%127] = AD_Value[0]*temp;
		val[count%127] = (Val[(count)%127]+Val[(count-1)%127])/2;
		OLED_ShowNum(1,13,AD_Value[0]*temp,4);
		OLED_Arr_Update(val[count%127]);
		OLED_Update();
		
		uint16_t threshold = Compare_Number();

    uint16_t now  = val[count & 0x7F];
    uint16_t prev = val[(count - 1) & 0x7F];
		
		//连续上升次数
		if(now > prev + 3)
        rise_count++;
    else
        rise_count = 0;

		//5s 没有跳动则心率为零
		if((current_time - last_peak_time) > 5)
        bpm_instant = 0;
		
		//检测到跳动，计算心率
		if((!in_peak) && rise_count >= MIN_RISE_SAMPLES && now > threshold )
    {
        if(last_peak_time > 0)
        {
            float dt = current_time - last_peak_time;
            if(dt > 0.35)
                bpm_instant = 60 / dt;
        }
        last_peak_time = current_time;
				in_peak = 1;
        rise_count = 0;    
				biaoji = !biaoji;
    }
		
		if(in_peak && now < prev)
		{
				in_peak = 0;
		}
		OLED_ShowNum(1, 1, bpm_instant,3);
		
		if(biaoji){GPIO_ResetBits(GPIOC,GPIO_Pin_13);}else{GPIO_SetBits(GPIOC,GPIO_Pin_13);}
		if((bpm_instant < 140) && (bpm_instant > 50)){GPIO_SetBits(GPIOC,GPIO_Pin_14);GPIO_SetBits(GPIOC,GPIO_Pin_15);}else{GPIO_ResetBits(GPIOC,GPIO_Pin_14);GPIO_ResetBits(GPIOC,GPIO_Pin_15);}
		
    TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}