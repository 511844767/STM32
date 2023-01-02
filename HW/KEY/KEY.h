#ifndef KEY_H__
#define KEY_H__

typedef enum{
    KEY_NONE_PRESS = 0,
    KEY0_PRESS, 
    KEY1_PRESS, 
    KEYUP_PRESS, 
} KEY_STATUS_t;

typedef enum{
    KEY_SCAN_CONTINUE = 0,
    KEY_SCAN_ONCE,
} KEY_SCAN_MODE_t;

void KEY_Init();
KEY_STATUS_t KEY_Scan(KEY_SCAN_MODE_t mode);


#endif