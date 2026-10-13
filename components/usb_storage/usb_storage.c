#include "usb_storage.h"
#include <stdlib.h>
#include "esp_log.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"
#include "tinyusb.h"
#include "tinyusb_default_config.h"
#include "tinyusb_msc.h"
#include "usb_phy_restore.h"

#define TF_CLK 41
#define TF_CMD 39
#define TF_D0 40
static const char *TAG = "usb_storage";
static usb_storage_state_t state = USB_STORAGE_IDLE;
static sdmmc_card_t *card;
static tinyusb_msc_storage_handle_t storage;
static bool sd_host_initialized;
static bool usb_driver_installed;
static bool msc_driver_installed;
static void on_msc_event(tinyusb_msc_storage_handle_t handle, tinyusb_msc_event_t *event, void *arg) {
    (void)arg;
    if (handle != storage || state != USB_STORAGE_EXPORTED) return;
    if (event->id == TINYUSB_MSC_EVENT_MOUNT_COMPLETE &&
        event->mount_point == TINYUSB_MSC_STORAGE_MOUNT_APP) {
        state = USB_STORAGE_SAFE_TO_EXIT;
        ESP_LOGI(TAG, "Mac eject confirmed: storage transferred to APP");
    } else if (event->id == TINYUSB_MSC_EVENT_MOUNT_FAILED &&
               event->mount_point == TINYUSB_MSC_STORAGE_MOUNT_APP) {
        ESP_LOGE(TAG, "Host eject received but APP mount failed, stay on USB page");
    }
}

usb_storage_state_t usb_storage_get_state(void) { return state; }
bool usb_storage_can_exit(void) {
    // Host must be safely ejected; USB-disconnect alone is not a safe eject.
    return state == USB_STORAGE_IDLE || state == USB_STORAGE_SAFE_TO_EXIT ||
           state == USB_STORAGE_ERROR;
}
static void cleanup(void) {
    if (storage) {
        esp_err_t err = tinyusb_msc_delete_storage(storage);
        if (err == ESP_OK) storage = NULL;
        else ESP_LOGE(TAG,"MSC storage deletion failed: %s",esp_err_to_name(err));
    }
    if (usb_driver_installed && !storage) {
        esp_err_t err=tinyusb_driver_uninstall();
        if (err == ESP_OK) usb_driver_installed=false;
    }
    if (msc_driver_installed && !storage && !usb_driver_installed) {
        if (tinyusb_msc_uninstall_driver() == ESP_OK) msc_driver_installed=false;
    }
    if (!storage && !usb_driver_installed && !msc_driver_installed && sd_host_initialized) {
        sdmmc_host_deinit();
        sd_host_initialized=false;
        free(card);card=NULL;
    }
}
esp_err_t usb_storage_enter(void) {
    if (state != USB_STORAGE_IDLE) return ESP_ERR_INVALID_STATE;
    state = USB_STORAGE_STARTING;
    // The Launcher does not mount a local FATFS before exporting storage,
    // so TinyUSB owns the card exclusively from the first sector access.
    sdmmc_host_t host=SDMMC_HOST_DEFAULT();
    sdmmc_slot_config_t slot=SDMMC_SLOT_CONFIG_DEFAULT();
    slot.width=1;
    slot.clk=TF_CLK;slot.cmd=TF_CMD;slot.d0=TF_D0;
    slot.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
    card=calloc(1,sizeof(*card));
    if (!card) { state=USB_STORAGE_ERROR; return ESP_ERR_NO_MEM; }
    esp_err_t err=sdmmc_host_init();
    if (err != ESP_OK) goto failed;
    sd_host_initialized=true;
    err=sdmmc_host_init_slot(host.slot,&slot);
    if (err != ESP_OK) goto failed;
    err=sdmmc_card_init(&host,card);
    if (err != ESP_OK) goto failed;
    tinyusb_msc_driver_config_t driver_config = { 0 };
    err = tinyusb_msc_install_driver(&driver_config);
    if (err != ESP_OK) goto failed;
    msc_driver_installed = true;
    err = tinyusb_msc_set_storage_callback(on_msc_event, NULL);
    if (err != ESP_OK) goto failed;
    tinyusb_msc_storage_config_t config = {
        .medium.card = card,
        .fat_fs = { .do_not_format = true },
        .mount_point = TINYUSB_MSC_STORAGE_MOUNT_USB,
    };
    err=tinyusb_msc_new_storage_sdmmc(&config,&storage);
    if (err != ESP_OK) goto failed;
    tinyusb_config_t tusb=TINYUSB_DEFAULT_CONFIG(NULL);
    err=tinyusb_driver_install(&tusb);
    if (err != ESP_OK) goto failed;
    usb_driver_installed=true;
    state=USB_STORAGE_EXPORTED;
    ESP_LOGI(TAG,"TF card exported over native USB MSC. Eject on Mac before Back.");
    return ESP_OK;
failed:
    ESP_LOGE(TAG,"MSC startup failed: %s",esp_err_to_name(err));
    cleanup();
    state=USB_STORAGE_ERROR;
    return err;
}
esp_err_t usb_storage_exit(void) {
    if (!usb_storage_can_exit()) return ESP_ERR_INVALID_STATE;
    cleanup();
    if (storage || usb_driver_installed || msc_driver_installed) return ESP_ERR_INVALID_STATE;
    state=USB_STORAGE_IDLE;
    // On S3 USB-OTG and USB Serial/JTAG share the internal PHY. Restore
    // only when TinyUSB and the SD storage handles have been released.
    esp_err_t phy_err = usb_phy_restore_serial_jtag();
    if (phy_err != ESP_OK) {
        ESP_LOGW(TAG, "MSC closed but USB Serial/JTAG recovery unconfirmed: %s", esp_err_to_name(phy_err));
    }
    return ESP_OK;
}
