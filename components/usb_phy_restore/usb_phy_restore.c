#include "usb_phy_restore.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_private/periph_ctrl.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/usb_serial_jtag_reg.h"
#include "soc/periph_defs.h"
#include "hal/usb_serial_jtag_ll.h"
#include "soc/usb_serial_jtag_struct.h"

static const char *TAG = "usb_phy_restore";

esp_err_t usb_phy_restore_serial_jtag(void)
{
#if !CONFIG_IDF_TARGET_ESP32S3
    return ESP_ERR_NOT_SUPPORTED;
#else
    // Called after tinyusb_driver_uninstall only. Based on Espressif's
    // Arduino-ESP32 usb_switch_to_cdc_jtag() controller hand-off.
    // A CPU restart alone does not restore the physical controller.
    ESP_LOGI(TAG, "Resetting USB-OTG and handing PHY to USB Serial/JTAG");
    periph_module_reset(PERIPH_USB_MODULE);
    periph_module_disable(PERIPH_USB_MODULE);

    CLEAR_PERI_REG_MASK(RTC_CNTL_USB_CONF_REG,
                        RTC_CNTL_SW_HW_USB_PHY_SEL |
                        RTC_CNTL_SW_USB_PHY_SEL |
                        RTC_CNTL_USB_PAD_ENABLE);
    CLEAR_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG,
                        USB_SERIAL_JTAG_PHY_SEL |
                        USB_SERIAL_JTAG_USB_PAD_ENABLE);

    // Give macOS a visible USB disconnect before reattaching the JTAG PHY.
    // These are ESP32-S3's fixed internal USB D-/D+ pads.
    gpio_config_t pins = {
        .pin_bit_mask = (1ULL << GPIO_NUM_19) | (1ULL << GPIO_NUM_20),
        .mode = GPIO_MODE_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&pins);
    if (err != ESP_OK) return err;
    gpio_set_level(GPIO_NUM_19, 0);
    gpio_set_level(GPIO_NUM_20, 0);
    vTaskDelay(pdMS_TO_TICKS(50));

    // Same pull override preparation as Espressif's hardware CDC+JTAG path.
    const usb_serial_jtag_pull_override_vals_t pull = {
        .dp_pu = 1, .dm_pu = 0, .dp_pd = 0, .dm_pd = 0
    };
    usb_serial_jtag_ll_phy_enable_pull_override(&pull);
    usb_serial_jtag_ll_phy_disable_pull_override();

    usb_serial_jtag_ll_disable_intr_mask(USB_SERIAL_JTAG_LL_INTR_MASK);
    usb_serial_jtag_ll_clr_intsts_mask(USB_SERIAL_JTAG_LL_INTR_MASK);
    usb_serial_jtag_ll_ena_intr_mask(USB_SERIAL_JTAG_INTR_BUS_RESET);
    SET_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG,
                      USB_SERIAL_JTAG_USB_PAD_ENABLE);

    const int64_t deadline = esp_timer_get_time() + 1500000;
    bool bus_reset = false;
    while (esp_timer_get_time() < deadline) {
        uint32_t flags = usb_serial_jtag_ll_get_intsts_mask();
        if (flags & USB_SERIAL_JTAG_INTR_BUS_RESET) {
            usb_serial_jtag_ll_clr_intsts_mask(flags);
            bus_reset = true;
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    usb_serial_jtag_ll_disable_intr_mask(USB_SERIAL_JTAG_LL_INTR_MASK);
    if (!bus_reset) {
        ESP_LOGW(TAG, "USB Serial/JTAG bus reset not observed (host may re-enumerate later)");
        return ESP_ERR_TIMEOUT;
    }
    ESP_LOGI(TAG, "USB Serial/JTAG bus reset observed; check Mac serial device");
    return ESP_OK;
#endif
}
