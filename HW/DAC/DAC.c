#include"stdio.h"
#include"stm32f10x.h"
#include"stm32f10x_dac.h"
#include"delay.h"
#include"KEY.h"

__IO uint16_t DAC_Value = 0;

/* 初始化DAC输出（DAC1对应PA4） */
void DAC_Single_Init(){
    /* 初始化GPIO */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef gpioDef;
    gpioDef.GPIO_Mode   = GPIO_Mode_AIN;  // 注意DAC要求GPIO配置为模拟输入
    gpioDef.GPIO_Pin    = GPIO_Pin_4;
    gpioDef.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioDef);
    GPIO_SetBits(GPIOA, GPIO_Pin_4);            // 软件触发输出
    /* 初始化DAC */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);
    DAC_InitTypeDef dacDef;
    dacDef.DAC_LFSRUnmask_TriangleAmplitude = DAC_LFSRUnmask_Bit0;      // 输出三角波参数（没用到）
    dacDef.DAC_OutputBuffer                 = DAC_OutputBuffer_Disable; // 不使能输出缓存（输出缓存使DAC输出有更好的驱动能力，但会降低精度，且无法输出0电压）
    dacDef.DAC_Trigger                      = DAC_Trigger_None;         // DAC触发方式（软件触发）
    dacDef.DAC_WaveGeneration               = DAC_WaveGeneration_None;  // 输出三角波参数（没用到）
    DAC_Init(DAC_Channel_1, &dacDef);
    /* 使能DAC */
    DAC_Cmd(DAC_Channel_1, ENABLE);
    /* 设置DAC的电压 */
    DAC_SetChannel1Data(DAC_Align_12b_R, 0);    // 12bit右对齐
}

extern void ADC_Single_Init();  // 注意ADC选择用DMA读取
extern __IO uint16_t ADC_Value;
extern void USART1_Init();

void DAC_Single_Run(){
    USART1_Init();
    KEY_Init();
    delay_init();
    ADC_Single_Init();
    DAC_Single_Init();

    while(1){
        KEY_STATUS_t key = KEY_Scan(KEY_SCAN_ONCE);
        if(key == KEY0_PRESS){  // -100
            DAC_Value = (DAC_Value <= 100)? 0 : (DAC_Value - 100); 
            DAC_SetChannel1Data(DAC_Align_12b_R, DAC_Value);
            delay_ms(5);
            printf("DAC_Value = %.3fV\tADC_Value = %.3fV\r\n", 3.3f * ((float)DAC_Value / 4095), 3.3f * ((float)ADC_Value / 4095));
        }else if(key == KEY1_PRESS){
            DAC_Value = (DAC_Value >= 4095 - 100)? 4095 : (DAC_Value + 100); 
            DAC_SetChannel1Data(DAC_Align_12b_R, DAC_Value); 
            delay_ms(5);
            printf("DAC_Value = %.3fV\tADC_Value = %.3fV\r\n", 3.3f * ((float)DAC_Value / 4095), 3.3f * ((float)ADC_Value / 4095));
        }
    }
}