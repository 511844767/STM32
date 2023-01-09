#include"stdio.h"
#include"stm32f10x.h"
#include"stm32f10x_adc.h"
#include"delay.h"

/**
 * @brief 1: 用DMA读取ADC数据；0: 用中断读取ADC数据
 * @note DMA是异步读取，中断是同步读取; 
 *  双ADC工作时通过ADC_Mode选择工作顺序和触发方式
 *  双ADC只能是ADC1和ADC2，而ADC3是一个完全独立的ADC外设
 *  单ADC多通道手册上推荐用DMA读取数据
 */
#define USE_DMA 1

/* Q12定点数 */
__IO uint16_t ADC_Value = 0;

/**
 * @brief 单ADC初始化
 * 
 */
void ADC_Single_Init(){
    /* 配置GPIO引脚 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    GPIO_InitTypeDef gpioDef;
    gpioDef.GPIO_Mode   = GPIO_Mode_AIN;
    gpioDef.GPIO_Pin    = GPIO_Pin_1;
    gpioDef.GPIO_Speed  = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &gpioDef);
    /* 配置ADC外设 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
    ADC_InitTypeDef adcDef;
    adcDef.ADC_ContinuousConvMode   = ENABLE;                       // 持续检测模式
    adcDef.ADC_DataAlign            = ADC_DataAlign_Right;          // 12bit数据右对齐到16bit寄存器中
    adcDef.ADC_ExternalTrigConv     = ADC_ExternalTrigConv_None;    // 检测触发源，可以是定时器触发、gpio触发、软件触发（None表示软件触发）
    adcDef.ADC_Mode                 = ADC_Mode_Independent;         // 单ADC下选择独立模式
    adcDef.ADC_NbrOfChannel         = 1;                            // 通道数量
    adcDef.ADC_ScanConvMode         = DISABLE;                      // 单通道选择不扫描
    ADC_Init(ADC1, &adcDef);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);   // 配置时钟频率，72MHz / 6 = 12MHz
    ADC_RegularChannelConfig(ADC1, ADC_Channel_1, 1, ADC_SampleTime_239Cycles5);  // 配置ADC通道，Rank表示第几个采样（单通道为1），ADC_SampleTime为采样时间（采样时间太短会影响精度）

#if(USE_DMA)
    /**
     * @brief n通道连续采样时，n个通道扫描完一轮后，会产生dma请求，此时就能用dma读取一轮的数据了
     */
    DMA_DeInit(DMA1_Channel1);
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);
    DMA_InitTypeDef dmaDef;
    dmaDef.DMA_PeripheralBaseAddr   = (uint32_t)(&(ADC1->DR));                  // ADC数据寄存器地址
    dmaDef.DMA_MemoryBaseAddr       = (uint32_t)(&ADC_Value);                   // 内存地址
    dmaDef.DMA_DIR                  = DMA_DIR_PeripheralSRC;                    // 传输方向，P->M
    dmaDef.DMA_BufferSize           = 1;                                        // 数据大小(CNDTR寄存器)
    dmaDef.DMA_PeripheralInc        = DMA_PeripheralInc_Disable;                // 地址自增
    dmaDef.DMA_MemoryInc            = DMA_MemoryInc_Disable;                    // 地址自增
    dmaDef.DMA_PeripheralDataSize   = DMA_PeripheralDataSize_HalfWord;          // 数据大小
    dmaDef.DMA_MemoryDataSize       = DMA_MemoryDataSize_HalfWord;              // 数据大小
    dmaDef.DMA_Mode                 = DMA_Mode_Circular;                        // 传输模式（单次传输/循环传输）
    dmaDef.DMA_Priority             = DMA_Priority_Medium;                      // 传输优先级
    dmaDef.DMA_M2M                  = DMA_M2M_Disable;                          // 关闭M->M的传输方向
    DMA_Init(DMA1_Channel1, &dmaDef);
    DMA_Cmd(DMA1_Channel1, ENABLE);
    ADC_DMACmd(ADC1, ENABLE);   // 开启循环传输
#else
    /* 配置ADC中断 */
    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE); // EOC通道转换完成中断
    NVIC_InitTypeDef nvicDef;
    nvicDef.NVIC_IRQChannel                     = ADC1_2_IRQn;
    nvicDef.NVIC_IRQChannelCmd                  = ENABLE;
    nvicDef.NVIC_IRQChannelPreemptionPriority   = 2;
    nvicDef.NVIC_IRQChannelSubPriority          = 2;
    NVIC_Init(&nvicDef);
#endif /* USE_DMA */

    /* 使能ADC */
    ADC_Cmd(ADC1, ENABLE);
    /* 校准ADC */
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));  // 等待校准完成
    /* 使能软件触发ADC */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
}

#if(!USE_DMA)
/**
 * @brief 中断服务函数
 * @note n个通道连续采样时，如果用中断读取结果，每次扫描一轮就会进入n次中断服务函数，此时应该用状态机标识当前是第几个通道
 *      多通道连续采样用dma读取数据会更加方便和安全，因为dma是异步的，不容易被别的程序干扰
 */
void ADC1_2_IRQHandler(void){
    if(ADC_GetITStatus(ADC1, ADC_IT_EOC)){
        ADC_Value = ADC_GetConversionValue(ADC1);   // 读取当前通道的数值
        printf("ADC_Value = %.3f\r\n", 3.3f * ((float)ADC_Value / 4096));
        delay_s(1);
        ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
    }
}
#endif /* USE_DMA */

extern void USART1_Init();

void ADC_Single_Run(){
    /* 初始化 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    USART1_Init();
    delay_init();
    ADC_Single_Init();

#if(USE_DMA)
    /* 打印结果 */
    while(1){
        printf("ADC_Value = %.3f\r\n", 3.3f * ((float)ADC_Value / 4095));
        delay_s(1);
    }
#else
    /* 等待中断 */
    while(1);
#endif /* USE_DMA */
}