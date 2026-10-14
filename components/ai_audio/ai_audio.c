#include "ai_audio.h"
#include "driver/i2c_master.h"
#include "driver/i2s_tdm.h"
#include "esp_codec_dev.h"
#include "esp_codec_dev_defaults.h"
#include "esp_log.h"
#include <stdbool.h>
static const char *TAG="ai_audio";
static esp_codec_dev_handle_t playback;
static i2s_chan_handle_t tx;
static const audio_codec_data_if_t *data_if;
static const audio_codec_ctrl_if_t *ctrl_if;
static const audio_codec_gpio_if_t *gpio_if;
static const audio_codec_if_t *codec_if;
static uint8_t volume=90;
esp_err_t ai_audio_init(void) {
    if(playback) return ESP_OK;
    i2c_master_bus_handle_t bus=NULL;
    esp_err_t err=i2c_master_get_bus_handle(I2C_NUM_0,&bus);
    if(err!=ESP_OK || !bus) return err==ESP_OK ? ESP_ERR_INVALID_STATE : err;
    i2s_chan_config_t cfg=I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_0,I2S_ROLE_MASTER);
    cfg.auto_clear=true;cfg.dma_desc_num=3;cfg.dma_frame_num=128;
    err=i2s_new_channel(&cfg,&tx,NULL);
    if(err!=ESP_OK) return err;
    i2s_tdm_slot_mask_t slots=I2S_TDM_SLOT0|I2S_TDM_SLOT1|I2S_TDM_SLOT2|I2S_TDM_SLOT3;
    i2s_tdm_config_t tdm={
      .clk_cfg=I2S_TDM_CLK_DEFAULT_CONFIG(24000),
      .slot_cfg=I2S_TDM_PHILIPS_SLOT_DEFAULT_CONFIG(32,I2S_SLOT_MODE_STEREO,slots),
      .gpio_cfg={.mclk=GPIO_NUM_7,.bclk=GPIO_NUM_15,.ws=GPIO_NUM_46,.dout=GPIO_NUM_45,.din=GPIO_NUM_6}
    };
    tdm.slot_cfg.total_slot=4;
    err=i2s_channel_init_tdm_mode(tx,&tdm);
    if(err!=ESP_OK) return err;
    err=i2s_channel_enable(tx);
    if(err!=ESP_OK) return err;
    gpio_if=audio_codec_new_gpio();
    audio_codec_i2s_cfg_t di={.port=I2S_NUM_0,.tx_handle=tx,.rx_handle=NULL};
    data_if=audio_codec_new_i2s_data(&di);
    audio_codec_i2c_cfg_t ci={.port=I2C_NUM_0,.addr=ES8311_CODEC_DEFAULT_ADDR,.bus_handle=bus};
    ctrl_if=audio_codec_new_i2c_ctrl(&ci);
    if(!gpio_if||!data_if||!ctrl_if) return ESP_ERR_NO_MEM;
    es8311_codec_cfg_t cc={
      .codec_mode=ESP_CODEC_DEV_WORK_MODE_DAC,.ctrl_if=ctrl_if,
      .gpio_if=gpio_if,.pa_pin=-1,.use_mclk=true,.hw_gain={.pa_gain=6}
    };
    codec_if=es8311_codec_new(&cc);
    if(!codec_if) return ESP_ERR_NO_MEM;
    esp_codec_dev_cfg_t dc={.codec_if=codec_if,.data_if=data_if,.dev_type=ESP_CODEC_DEV_TYPE_OUT};
    playback=esp_codec_dev_new(&dc);
    if(!playback) return ESP_ERR_NO_MEM;
    esp_codec_dev_sample_info_t fs={.sample_rate=24000,.channel=2,.bits_per_sample=16};
    if(esp_codec_dev_open(playback,&fs)!=ESP_CODEC_DEV_OK) return ESP_FAIL;
    ESP_LOGI(TAG,"ES8311 opened");
    return ai_audio_set_volume(volume);
}
esp_err_t ai_audio_set_volume(uint8_t v) {
    if(v>100) v=100;
    if(!playback) return ESP_ERR_INVALID_STATE;
    if(esp_codec_dev_set_out_vol(playback,v)!=ESP_CODEC_DEV_OK) return ESP_FAIL;
    volume=v;return ESP_OK;
}
uint8_t ai_audio_get_volume(void){return volume;}
