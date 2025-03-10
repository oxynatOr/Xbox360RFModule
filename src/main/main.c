#include <stdio.h>
#include "esp_system.h"
#include "driver/gpio.h"
#include "freeRTOS/freeRTOS.h"
#include "freeRTOS/task.h"
#include "esp_sleep.h"
#include "esp_log.h"

#include "rfmodule.h"
#include "button.h"




bool sync_enable = false;
bool turn_off_controllers = false;

button_event_t ev;
QueueHandle_t button_events;

esp_sleep_wakeup_cause_t wakeup_cause;


void setup_(void) 
{  
  esp_log_level_set("*", ESP_LOG_INFO);

  ESP_LOGI("INIT","Set GPIOs");    
  //button_events = button_init(PIN_BIT(btn_sw1_pin) | PIN_BIT(extbtn_sw2));  

  ESP_LOGI("INIT-GPIO","Set Button#1");
  ESP_ERROR_CHECK(gpio_pullup_en(btn_sw1_pin));
  ESP_ERROR_CHECK(gpio_pulldown_dis(btn_sw1_pin));  

  ESP_LOGI("INIT-GPIO","Set Button#2");      
  ESP_ERROR_CHECK(gpio_pullup_en(extbtn_sw2));
  ESP_ERROR_CHECK(gpio_pulldown_dis(extbtn_sw2));  
  
  ESP_LOGI("INIT-GPIO","Set GPIO WeakeUp");      
  ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(PIN_BIT(btn_sw1_pin), ESP_GPIO_WAKEUP_GPIO_LOW));
  ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(PIN_BIT(extbtn_sw2), ESP_GPIO_WAKEUP_GPIO_LOW));

  ESP_LOGI("INIT-GPIO","Set LEDs");    
  ESP_ERROR_CHECK(gpio_set_direction(intLED, GPIO_MODE_OUTPUT)); 
  ESP_ERROR_CHECK(gpio_set_direction(extLED_R, GPIO_MODE_OUTPUT)); 
  ESP_ERROR_CHECK(gpio_set_direction(extLED_G, GPIO_MODE_OUTPUT)); 
  ESP_ERROR_CHECK(gpio_set_direction(extLED_B, GPIO_MODE_OUTPUT));   

  ESP_LOGI("INIT-GPIO","Set Data-Clock");
  initPin(clock_pin_SEL);//, GPIO_PULLUP_ENABLE, GPIO_PULLDOWN_DISABLE);
  initPin(data_pin_SEL);


  button_events = button_init(1ULL<<btn_sw1_pin | 1ULL<<extbtn_sw2);    
  wakeup_cause = esp_sleep_get_wakeup_cause(); 

  _leds_White();
  //vTaskDelay(1000 / portTICK_PERIOD_MS);
}


void app_main(void)
{  
  setup_();    
  
  if (wakeup_cause != ESP_SLEEP_WAKEUP_GPIO) { 
    ESP_LOGV("BOOT","init RF Module\n");
    _led_Yellow();    
    sendData(0);    
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    ESP_LOGV("BOOT","send Init Animation\n");
    _led_Yellow();
    sendData(2);
  }  
  if (wakeup_cause == ESP_SLEEP_WAKEUP_GPIO) {
    ESP_LOGI("WAKE-UP", "GPIO WakeUp\n");
    
    while (true) {
      if (xQueueReceive(button_events, &ev, 1000/portTICK_PERIOD_MS)) {     
          //ESP_LOGI("xQueueReceive", "Pin# %d || event: %d\n",ev.pin, ev.event);

          if (((ev.pin == btn_sw1_pin) || (ev.pin == extbtn_sw2)) && (ev.event == BUTTON_UP)) {
            if (sync_enable == true) {
              ESP_LOGI("-Event-","send Sync");
              sendData(3);
            } else if (turn_off_controllers == true) {
              ESP_LOGI("-Event-","turn off Pads");
              sendData(4);
            }      
            enterDeepSleep();
          }                      
  
        if (((ev.pin == btn_sw1_pin) || (ev.pin == extbtn_sw2)) && (ev.event == BUTTON_DOWN)) {       
          sync_enable = true;
          turn_off_controllers = false;
        }          
    
        if (((ev.pin == btn_sw1_pin) || (ev.pin == extbtn_sw2)) && (ev.event == BUTTON_HELD)) {
          sync_enable = false;
          turn_off_controllers = true;
        }  
      }
    } 
      /*
    while (true) {
      if (xQueueReceive(button_events, &ev, 1000/portTICK_PERIOD_MS)) {

        if (((ev.pin == extbtn_sw2)) && (ev.event == BUTTON_DOWN)) { 
          sendData(9);
        }

        if (((ev.pin == btn_sw1_pin)) && (ev.event == BUTTON_DOWN)) { 
          sendData(10);
        }

        if (((ev.pin == extbtn_sw2)) && (ev.event == BUTTON_HELD)) { 
          sendData(2);
        }

        if (((ev.pin == btn_sw1_pin)) && (ev.event == BUTTON_HELD)) { 
          sendData(1);
        }        

      }
    }
    */
  }
  enterDeepSleep();
}