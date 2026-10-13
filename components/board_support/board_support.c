#include "board_support.h"
#include <string.h>
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_timer.h"
#include "esp_heap_caps.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "driver/i2c_master.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_axs15231b.h"
#include "esp_io_expander_tca9554.h"

#define DEFAULT_BRIGHTNESS_PERCENT 20
#define BACKLIGHT_GPIO 42
#define PHYS_W 172
#define PHYS_H 640
#define LOG_W 640
#define LOG_H 172
#define BUF_BYTES (LOG_W * LOG_H * 2)
#define FLUSH_ROWS 16
#define FLUSH_BYTES (PHYS_W * FLUSH_ROWS * 2)
static const char *TAG="v2_board";
static esp_lcd_panel_handle_t panel;
static esp_lcd_panel_io_handle_t panel_io;
static esp_io_expander_handle_t expander;
static i2c_master_dev_handle_t touch_dev;
static lv_display_t *display;
static uint8_t *frame_buf;
static uint8_t *rot_buf;
static uint8_t *dma_buf;
static bool ready;
static uint32_t flush_errors;
static SemaphoreHandle_t flush_done;

static const axs15231b_lcd_init_cmd_t lcd_init_cmds[] = {
    {0x11, (uint8_t[]){0}, 0, 100},
    {0x29, (uint8_t[]){0}, 0, 100},
};
static bool tx_done(esp_lcd_panel_io_handle_t io, esp_lcd_panel_io_event_data_t *data, void *ctx) {
    (void)io;(void)data;(void)ctx;
    BaseType_t wake=pdFALSE;
    xSemaphoreGiveFromISR(flush_done,&wake);
    return wake==pdTRUE;
}
static void flush_cb(lv_display_t *d,const lv_area_t *area,uint8_t *px) {
    // Full-frame LVGL render at logical 640x172; transpose into native 172x640.
    // Keep DMA copy alive until panel transaction finishes via synchronous waits.
    (void)area;
    lv_draw_sw_rgb565_swap(px,LOG_W*LOG_H);
    (void)d;
    uint32_t src_stride=lv_draw_buf_width_to_stride(LOG_W,LV_COLOR_FORMAT_RGB565);
    uint32_t dst_stride=lv_draw_buf_width_to_stride(PHYS_W,LV_COLOR_FORMAT_RGB565);
    lv_draw_sw_rotate(px,rot_buf,LOG_W,LOG_H,src_stride,dst_stride,
                      LV_DISPLAY_ROTATION_90,LV_COLOR_FORMAT_RGB565);
    // esp_lcd_panel_draw_bitmap is asynchronous; ensure the preceding DMA
    // transaction finished before overwriting dma_buf.
    for (int row=0; row<PHYS_H; row+=FLUSH_ROWS) {
        memcpy(dma_buf,rot_buf+row*PHYS_W*2,FLUSH_BYTES);
        esp_err_t err=esp_lcd_panel_draw_bitmap(panel,0,row,PHYS_W,row+FLUSH_ROWS,dma_buf);
        if(err!=ESP_OK) { if(flush_errors++<4) ESP_LOGE(TAG,"flush: %s",esp_err_to_name(err));break; }
        if(xSemaphoreTake(flush_done,pdMS_TO_TICKS(500))!=pdTRUE) {
            ESP_LOGE(TAG,"LCD QSPI DMA completion timeout");
            break;
        }
    }
    lv_display_flush_ready(d);
}
static void touch_read(lv_indev_t *indev,lv_indev_data_t *data) {
    (void)indev;
    uint8_t cmd[11]={0xb5,0xab,0xa5,0x5a,0,0,0,0x0e,0,0,0};
    uint8_t resp[32]={0};
    esp_err_t e=i2c_master_transmit_receive(touch_dev,cmd,sizeof(cmd),resp,sizeof(resp),50);
    if (e==ESP_OK && resp[1]>0 && resp[1]<5) {
        int x=((resp[4]&0x0f)<<8)|resp[5];
        int y=((resp[2]&0x0f)<<8)|resp[3];
        if (x>=PHYS_W) x=PHYS_W-1;
        if (y>=PHYS_H) y=PHYS_H-1;
        data->point.x=y;
        data->point.y=LOG_H-1-x;
        data->state=LV_INDEV_STATE_PRESSED;
    } else data->state=LV_INDEV_STATE_RELEASED;
}
static void tick(void *arg){(void)arg;lv_tick_inc(5);}
bool board_support_ready(void){return ready;}
void board_support_loop(void){if(ready) lv_timer_handler();}
static esp_err_t backlight_pwm_init(void) {
    // Waveshare V2 PWM backlight polarity is inverted: 255=off, 0=max.
    ledc_timer_config_t timer = {
        .speed_mode=LEDC_LOW_SPEED_MODE,
        .duty_resolution=LEDC_TIMER_8_BIT,
        .timer_num=LEDC_TIMER_3,
        .freq_hz=50000,
        .clk_cfg=LEDC_AUTO_CLK,
    };
    ESP_RETURN_ON_ERROR(ledc_timer_config(&timer), TAG, "backlight PWM timer");
    ledc_channel_config_t channel = {
        .gpio_num=BACKLIGHT_GPIO,
        .speed_mode=LEDC_LOW_SPEED_MODE,
        .channel=LEDC_CHANNEL_1,
        .intr_type=LEDC_INTR_DISABLE,
        .timer_sel=LEDC_TIMER_3,
        .duty=255 - (255 * DEFAULT_BRIGHTNESS_PERCENT / 100),
        .hpoint=0,
    };
    ESP_RETURN_ON_ERROR(ledc_channel_config(&channel), TAG, "backlight PWM channel");
    ESP_LOGI(TAG,"Boot backlight target=%d%% (PWM duty=%lu)",
             DEFAULT_BRIGHTNESS_PERCENT,(unsigned long)channel.duty);
    return ESP_OK;
}
esp_err_t board_support_init(void) {
    ESP_RETURN_ON_ERROR(backlight_pwm_init(),TAG,"backlight startup");
    lv_init();
    flush_done=xSemaphoreCreateBinary();
    if(!flush_done) return ESP_ERR_NO_MEM;
    i2c_master_bus_handle_t bus0,bus1;
    i2c_master_bus_config_t i0={
        .clk_source=I2C_CLK_SRC_DEFAULT,.i2c_port=I2C_NUM_0,
        .scl_io_num=48,.sda_io_num=47,.glitch_ignore_cnt=7,
        .flags.enable_internal_pullup=true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&i0,&bus0),TAG,"expander I2C");
    ESP_RETURN_ON_ERROR(esp_io_expander_new_i2c_tca9554(bus0,ESP_IO_EXPANDER_I2C_TCA9554_ADDRESS_000,&expander),TAG,"TCA9554");
    ESP_RETURN_ON_ERROR(esp_io_expander_set_dir(expander,(1ULL<<1)|(1ULL<<5),IO_EXPANDER_OUTPUT),TAG,"expander outputs");
    ESP_RETURN_ON_ERROR(esp_io_expander_set_level(expander,(1ULL<<1),0),TAG,"backlight off");
    ESP_RETURN_ON_ERROR(esp_io_expander_set_level(expander,(1ULL<<5),1),TAG,"LCD reset hi");
    i2c_master_bus_config_t i1={
        .clk_source=I2C_CLK_SRC_DEFAULT,.i2c_port=I2C_NUM_1,
        .scl_io_num=18,.sda_io_num=17,.glitch_ignore_cnt=7,
        .flags.enable_internal_pullup=true,
    };
    ESP_RETURN_ON_ERROR(i2c_new_master_bus(&i1,&bus1),TAG,"touch I2C");
    i2c_device_config_t td={.dev_addr_length=I2C_ADDR_BIT_LEN_7,
                              .device_address=0x3b,.scl_speed_hz=300000};
    ESP_RETURN_ON_ERROR(i2c_master_bus_add_device(bus1,&td,&touch_dev),TAG,"touch");
    spi_bus_config_t sp={.sclk_io_num=10,.data0_io_num=11,.data1_io_num=12,
        .data2_io_num=13,.data3_io_num=14,.max_transfer_sz=FLUSH_BYTES};
    ESP_RETURN_ON_ERROR(spi_bus_initialize(SPI3_HOST,&sp,SPI_DMA_CH_AUTO),TAG,"QSPI");
    esp_lcd_panel_io_spi_config_t io={
        .cs_gpio_num=9,.dc_gpio_num=-1,.spi_mode=3,.pclk_hz=40000000,
        .trans_queue_depth=10,.on_color_trans_done=tx_done,
        .lcd_cmd_bits=32,.lcd_param_bits=8,.flags.quad_mode=true};
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_io_spi(SPI3_HOST,&io,&panel_io),TAG,"panel IO");
    axs15231b_vendor_config_t vendor={.init_cmds=lcd_init_cmds,
        .init_cmds_size=sizeof(lcd_init_cmds)/sizeof(lcd_init_cmds[0])};
    vendor.flags.use_qspi_interface=1;
    esp_lcd_panel_dev_config_t pc={
        .reset_gpio_num=-1,.rgb_ele_order=LCD_RGB_ELEMENT_ORDER_RGB,
        .bits_per_pixel=16,.vendor_config=&vendor};
    ESP_RETURN_ON_ERROR(esp_lcd_new_panel_axs15231b(panel_io,&pc,&panel),TAG,"panel");
    ESP_RETURN_ON_ERROR(esp_io_expander_set_level(expander,1ULL<<5,1ULL<<5),TAG,"reset high");
    vTaskDelay(pdMS_TO_TICKS(30));
    ESP_RETURN_ON_ERROR(esp_io_expander_set_level(expander,1ULL<<5,0),TAG,"reset low");
    vTaskDelay(pdMS_TO_TICKS(250));
    ESP_RETURN_ON_ERROR(esp_io_expander_set_level(expander,1ULL<<5,1ULL<<5),TAG,"reset release");
    vTaskDelay(pdMS_TO_TICKS(30));
    ESP_RETURN_ON_ERROR(esp_lcd_panel_init(panel),TAG,"panel init");
    ESP_LOGI(TAG,"Before LVGL buffers: PSRAM=%u, internal=%u, largest PSRAM=%u",
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
             (unsigned)heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
             (unsigned)heap_caps_get_largest_free_block(MALLOC_CAP_SPIRAM));
    if (heap_caps_get_free_size(MALLOC_CAP_SPIRAM) < BUF_BYTES*2) {
        ESP_LOGE(TAG,"PSRAM unavailable or insufficient; enable CONFIG_SPIRAM and rebuild");
        return ESP_ERR_NO_MEM;
    }
    frame_buf=heap_caps_malloc(BUF_BYTES,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    rot_buf=heap_caps_malloc(BUF_BYTES,MALLOC_CAP_SPIRAM|MALLOC_CAP_8BIT);
    dma_buf=heap_caps_malloc(FLUSH_BYTES,MALLOC_CAP_DMA|MALLOC_CAP_INTERNAL);
    if(!frame_buf||!rot_buf||!dma_buf) {
        ESP_LOGE(TAG,"Frame allocations failed: fb=%p rotate=%p dma=%p",frame_buf,rot_buf,dma_buf);
        return ESP_ERR_NO_MEM;
    }
    display=lv_display_create(LOG_W,LOG_H);
    lv_display_set_color_format(display,LV_COLOR_FORMAT_RGB565);
    lv_display_set_buffers(display,frame_buf,NULL,BUF_BYTES,LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(display,flush_cb);
    // Logical screen remains 640x172; flush rotates pixels to native 172x640.
    lv_indev_t *indev=lv_indev_create();
    lv_indev_set_type(indev,LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev,touch_read);
    const esp_timer_create_args_t args={.callback=tick,.name="lv_tick"};
    esp_timer_handle_t timer;
    ESP_RETURN_ON_ERROR(esp_timer_create(&args,&timer),TAG,"tick timer");
    ESP_RETURN_ON_ERROR(esp_timer_start_periodic(timer,5000),TAG,"start tick");
    ESP_RETURN_ON_ERROR(esp_io_expander_set_level(expander,1ULL<<1,1ULL<<1),TAG,"backlight on");
    ready=true;
    ESP_LOGI(TAG,"V2 display/touch started, 640x172, PSRAM framebuffer");
    return ESP_OK;
}
