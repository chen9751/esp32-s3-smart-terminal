#include "ai_key.h"
#include "driver/gpio.h"
#include "esp_timer.h"
static bool sample,stable,held,click_event,long_event;static int64_t changed,pressed;
void ai_key_init(void){gpio_config_t c={.pin_bit_mask=1ULL<<GPIO_NUM_0,.mode=GPIO_MODE_INPUT,.pull_up_en=GPIO_PULLUP_ENABLE};gpio_config(&c);sample=stable=gpio_get_level(GPIO_NUM_0)==0;changed=esp_timer_get_time();}
static void scan(void){int64_t now=esp_timer_get_time();bool raw=gpio_get_level(GPIO_NUM_0)==0;if(raw!=sample){sample=raw;changed=now;}if(sample!=stable&&now-changed>=30000){stable=sample;if(stable){pressed=now;held=false;}else if(!held&&now-pressed>=40000&&now-pressed<800000)click_event=true;}if(stable&&!held&&now-pressed>=800000){held=true;long_event=true;}}
bool ai_key_poll_short(void){scan();bool v=click_event;click_event=false;return v;}
bool ai_key_poll_long(void){scan();bool v=long_event;long_event=false;return v;}
