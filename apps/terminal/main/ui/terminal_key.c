#include "terminal_key.h"
#include "driver/gpio.h"
#include "esp_timer.h"
static bool sample,stable,held;
static int64_t changed,pressed;
void terminal_key_init(void){
 gpio_config_t c={.pin_bit_mask=1ULL<<GPIO_NUM_0,.mode=GPIO_MODE_INPUT,
 .pull_up_en=GPIO_PULLUP_ENABLE,.pull_down_en=GPIO_PULLDOWN_DISABLE};
 gpio_config(&c);sample=stable=(gpio_get_level(GPIO_NUM_0)==0);changed=esp_timer_get_time();
}
void terminal_key_poll(bool *short_press,bool *long_press){
 *short_press=false;*long_press=false;
 int64_t now=esp_timer_get_time();bool raw=gpio_get_level(GPIO_NUM_0)==0;
 if(raw!=sample){sample=raw;changed=now;}
 if(sample!=stable&&now-changed>=30000){
  stable=sample;
  if(stable){pressed=now;held=false;}
  else if(!held&&now-pressed>=40000&&now-pressed<800000)*short_press=true;
 }
 if(stable&&!held&&now-pressed>=800000){held=true;*long_press=true;}
}
