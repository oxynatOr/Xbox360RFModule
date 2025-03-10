#include "driver/gpio.h"
#include "freeRTOS/freeRTOS.h"
#include "freeRTOS/task.h"
#include "esp_sleep.h"
#include "esp_log.h"

#include "rfmodule.h"
/**   Credits goes to: https://wiki.tkkrlab.nl/tkkrlab.nl/wiki/XBOX_360_RF_Module.html  */
RF_Command _rfcmds[] = {
  {"INIT"           ,{0,0,1,0,0,0,0,1,0,0},0x084},    //*00 Initializes the LEDs (needed before any other commands) and turns on the power LED 
  {"UNINIT"         ,{0,0,1,0,0,0,0,0,0,0},0x080},    //*01 Turns off the LED controller
  {"BOOT"           ,{0,0,1,0,0,0,0,1,0,1},0x085},    //*02 Same as INIT + Displays the XBOX 360 boot LED sequence (appears to only work once)
  {"SYNC"           ,{0,0,0,0,0,0,0,1,0,0},0x004},    //*03 Displays the XBOX 360 controller sync LED sequence
  {"PAD_OFF"        ,{0,0,0,0,0,0,1,0,0,1},0x009},    //*04 Turns off all controllers    
  {"ALL_GREEN_ON"   ,{0,0,1,0,1,0,1,1,1,1},0x0A1},    //*05 Sets the four green LEDs on
  {"ALL_GREEN_OFF"  ,{0,0,1,0,1,0,0,0,0,0},0x0A0},    //*06 Sets the four green LEDs off
  {"ALL_RED_ON"     ,{0,0,1,0,1,1,1,1,1,1},0x0B1},    //*07 Sets the four red LEDs on
  {"ALL_RED_OFF"    ,{0,0,1,0,1,1,0,0,0,0},0x0B0},    //*08 Sets the four red LEDs off
  {"ALL_AMBER_ON"   ,{0,0,1,1,1,0,0,0,0,1},0x0E1},    //*09 Sets all leds to amber colour
  {"CLEAR_ALL_LED"  ,{0,0,1,1,0,0,0,0,0,0},0x0C0},    //*10 Clears any error display (blinking red leds or orange solid)
  {"ALL_SLOW_RED"   ,{0,0,1,1,0,0,0,0,0,1},0x0C1},    //*11 Blinks all four red LEDs slow
  {"D1_SLOW_RED"    ,{0,0,1,1,0,0,0,0,1,0},0x0C2},    //*12 Blinks top left red LED slow
  {"D2_SLOW_RED"    ,{0,0,1,1,0,0,0,0,1,1},0x0C3},    //*13 Blinks top right red LED slow
  {"D3_SLOW_RED"    ,{0,0,1,1,0,0,0,1,0,0},0x0C4},    //*14 Blinks bottom right red LED slow
  {"D4_SLOW_RED"    ,{0,0,1,1,0,0,0,1,0,1},0x0C5},    //*15 Blinks top left red LED slow
  {"ALL_FAST_RED"   ,{0,0,1,1,0,1,0,0,1,1},0x0D1},    //*16 Blinks all four red LEDs fast  
  {"D1_FAST_RED"    ,{0,0,1,1,0,1,0,0,1,0},0x0D2},    //*17 Blinks all four red LEDs fast
  {"D2_FAST_RED"    ,{0,0,1,1,0,1,0,0,1,1},0x0D3},    //*18 Blinks top right red LED fast
  {"D3_FAST_RED"    ,{0,0,1,1,0,1,0,1,0,0},0x0D4},    //*19 Blinks bottom right red LED fast
  {"D4_FAST_RED"    ,{0,0,1,1,0,1,0,1,0,1},0x0D5},    //*20 Blinks top left red LED fast
  {"RF_POS_H"       ,{0,0,0,0,0,0,0,1,1,1},0x011},    //*21 Configures the module horizontal
  {"RF_POS_V"       ,{0,0,0,0,0,0,0,1,1,0},0x010}     //*22 Configures the module vertical 
};  


void initPin(unsigned long long pin_select) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_POSEDGE;
    io_conf.mode = GPIO_MODE_INPUT;      
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pin_bit_mask = pin_select;
    gpio_config(&io_conf);
  }

  void enterDeepSleep(void) {
    __ledsOff();  
    ESP_LOGI("", "Entering deep sleep\n");
    esp_deep_sleep_start(); 
  }

  void xbox_send_word(int cmd_index) {
    ESP_ERROR_CHECK(gpio_set_direction(data_pin, GPIO_MODE_OUTPUT));    
    ESP_ERROR_CHECK(gpio_set_level(data_pin, 0));     
    uint32_t bit_ptr;
    uint32_t edge_detect_buf = 0;
      for (bit_ptr = 10; bit_ptr >= 1; bit_ptr--){
        /* Wait for clock to go low */
        edge_detect_buf = 0;
        while (edge_detect_buf == gpio_get_level(clock_pin));
        /* Set data to value */        
        gpio_set_level(data_pin, ( _rfcmds[cmd_index].hex & (1 << (bit_ptr - 1)) ) ? data_pin : 0);          //not wokring :(
        /* Wait for clock to go high */
        edge_detect_buf = 0;
        while (edge_detect_buf == gpio_get_level(clock_pin));
      }
  }

  void sendData(int cmd_index) {  
    ESP_ERROR_CHECK(gpio_set_direction(data_pin, GPIO_MODE_OUTPUT));    
    ESP_ERROR_CHECK(gpio_set_level(data_pin, 0)); 
    int prev = 1;
    int roundof = 0;
    for(int i = 0; i < 10; i++){    
      _led_Magenta(); 
      while (prev == gpio_get_level(clock_pin)){      
        if (roundof >= maxClkRounds) { esp_deep_sleep_start();}
        vTaskDelay(1);
        roundof++;
        } 
      _led_Cyan();      
      prev = gpio_get_level(clock_pin);     
      gpio_set_level(data_pin, _rfcmds[cmd_index].bin[i]);    
      _led_Magenta(); 
      roundof = 0;
      while (prev == gpio_get_level(clock_pin) ){
        if (roundof >= maxClkRounds) { esp_deep_sleep_start();}
        vTaskDelay(1);
        roundof++;
      } 
      _led_Cyan();
      prev = gpio_get_level(clock_pin);
    }
    gpio_set_level(data_pin, 1);
    gpio_set_direction(data_pin, GPIO_MODE_INPUT);
    _led_Green();    
  }  
  
  void __ledsOff(void) {
    ESP_ERROR_CHECK(gpio_set_level(extLED_R, 0));
    ESP_ERROR_CHECK(gpio_set_level(extLED_G, 0));
    ESP_ERROR_CHECK(gpio_set_level(extLED_B, 0));
  }
    
  void _leds_White(void) {
    ESP_ERROR_CHECK(gpio_set_level(extLED_R, 1));
    ESP_ERROR_CHECK(gpio_set_level(extLED_G, 1));
    ESP_ERROR_CHECK(gpio_set_level(extLED_B, 1));
  }
  
  void _led_Yellow(void) {
    __ledsOff();
    ESP_ERROR_CHECK(gpio_set_level(extLED_R, 1));
    ESP_ERROR_CHECK(gpio_set_level(extLED_G, 1));
  }
  
  void _led_Magenta(void) {
    __ledsOff();
    ESP_ERROR_CHECK(gpio_set_level(extLED_R, 1));
    ESP_ERROR_CHECK(gpio_set_level(extLED_B, 1));
  }
  
  void _led_Cyan(void) {
    __ledsOff();
    ESP_ERROR_CHECK(gpio_set_level(extLED_G, 1));
    ESP_ERROR_CHECK(gpio_set_level(extLED_B, 1));
  }
  
  void _led_Red(void) {
    __ledsOff();
    ESP_ERROR_CHECK(gpio_set_level(extLED_R, 1));
  }
  
  void _led_Blue(void) {
    __ledsOff();
    ESP_ERROR_CHECK(gpio_set_level(extLED_B, 1));
  }
  
  void _led_Green(void) {
    __ledsOff();
    ESP_ERROR_CHECK(gpio_set_level(extLED_G, 1));  
  }