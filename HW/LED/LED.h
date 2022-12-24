#ifndef LED_H__
#define LED_H__

#include"sys.h"

#define LED0_ON()     PBout(5) = 0; 
#define LED0_OFF()    PBout(5) = 1; 
#define LED0_REVERT() PBout(5) = !PBout(5); 
#define LED1_ON()     PEout(5) = 0; 
#define LED1_OFF()    PEout(5) = 1; 
#define LED1_REVERT() PEout(5) = !PEout(5); 

void LED_Init_Hal(void);

#endif