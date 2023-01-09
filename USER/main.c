
/* 蜂鸣器实验 */
extern void BEEP_Bit();
/* 外部中断实验 */
extern void EXTI_Run();
/* 按键实验 */
extern void KEY_RUN();
/* LED实验 */
extern void LED_HAL();
extern void LED_Register();
extern void LED_Bit();
/* 串口实验 */
extern void USART1_Run();
/* 独立看门狗实验 */
extern void IWDG_Run();
/* 窗口看门狗实验 */
extern void WWDG_Run();
/* 定时器实验 */
extern void TIM3_Run();				// 定时器中断
extern void TIM3_PWMOut_Run();		// 定时器PWM输出
extern void TIM5_Cap_Run();			// 定时器输入捕获
/* TPAD电容按键实验 */
extern void TPAD_Run();
/* DMA实验 */
extern void DMA_USART_Run();
/* EEPROM实验 */
extern void I2C_Software_Run();		// 软件仿真I2C协议读写EEPROM
extern void I2C_Hardware_Run();		// 硬件I2C读写EEPROM
/* FLASH实验 */
extern void Flash_EX_Run();			// 外部flash读写测试
extern void Flash_EX_FatFs_Run();	// 外部flash安装FatFs文件系统
/* SD卡实验 */
extern void SDCard_Run();			// SD卡读写测试
extern void SDCard_Fats_Run();		// SD卡FatFs文件系统
/* LCD实验 */
extern void LCD_Run();				// LCD底层驱动实验
extern void Font_Download();		// 字体文件烧录实验（从SD卡烧录到Flash）
/* 触摸屏实验 */
extern void Touch_Run();
/* ADC转换实验 */
extern void ADC_Single_Run();		// 单ADC实验
/* DAC模拟电压输出实验 */
extern void DAC_Single_Run();		// 但DAC实验
/* CAN通讯实验 */
extern void CAN_Silent_LoopBack_Run();	// 静默回环测试

int main(){
	CAN_Silent_LoopBack_Run();
}