#ifndef RFMODULE_H
#define RFMODULE_H


#ifdef __cplusplus
extern "C" {
#endif


#include "esp_system.h"

#define maxClkRounds 500

#define extLED_B GPIO_NUM_3 
#define extLED_G GPIO_NUM_2
#define extLED_R GPIO_NUM_1  
#define btn_sw1_pin GPIO_NUM_4 
#define extbtn_sw2 GPIO_NUM_5
#define intLED GPIO_NUM_8

#define data_pin GPIO_NUM_6
#define data_pin_SEL  (1ULL<<data_pin)
#define clock_pin GPIO_NUM_7
#define clock_pin_SEL  (1ULL<<clock_pin)

typedef struct RF_Command {
    char name[90];
    int bin[10];
    uint32_t hex;
  } RF_Command;

 
  void initPin(unsigned long long pin_select);  
    
  void sendData(int cmd_index);
  void xbox_send_word(int cmd_index);
  void enterDeepSleep(void);

  void __ledsOff(void);   
  void _leds_White(void);  
  void _led_Yellow(void);  
  void _led_Magenta(void);  
  void _led_Cyan(void);  
  void _led_Red(void);
  void _led_Blue(void);  
  void _led_Green(void);


#ifdef __cplusplus
}
#endif

#endif