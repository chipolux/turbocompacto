// vim: foldmethod=marker:foldmarker={{{,}}}
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <driver/spi_master.h>
#include <driver/twai.h>
#include <esp_chip_info.h>
#include <esp_event.h>
#include <esp_flash.h>
#include <esp_log.h>
#include <esp_pm.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>
#include <inttypes.h>
#include <nvs_flash.h>
#include <string.h>

#define NOW_MS ((uint32_t)(xTaskGetTickCount() * portTICK_PERIOD_MS))
#define CAN_MESSAGE_SIZE 8

// build up 16 message before emitting via ble, should be 256 bytes
#define MESSAGE_BUF_SIZE 16

static const char *TAG = "main";

typedef struct {
    uint32_t ts;
    uint16_t id;
    uint16_t sz;
    uint8_t data[CAN_MESSAGE_SIZE];
} can_message_t; // should be 16 bytes each

typedef struct {
    uint32_t count; // oversized, but fills alignment
    can_message_t messages[MESSAGE_BUF_SIZE];
} can_message_buf_t;

// an actively filling buffer and a buffer being emitted
static can_message_buf_t MESSAGE_BUF_0 = {0};
// static can_message_buf_t MESSAGE_BUF_1 = {0};
static can_message_buf_t *MESSAGE_BUF = &MESSAGE_BUF_0;

void setup_chip()
{ // {{{
    /* load chip info, flash size, etc. */
    esp_chip_info_t chip_info;
    uint32_t flash_size = 0;
    esp_chip_info(&chip_info);
    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get flash size.");
        return;
    }

    /* print chip info */
    ESP_LOGI(
        TAG, "%s, %d core, WiFi%s%s%s, v%d.%d, %" PRIu32 "MB %s flash, %" PRIu32 "KB min free heap",
        CONFIG_IDF_TARGET, chip_info.cores, (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
        (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "",
        (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "",
        major_rev, minor_rev, flash_size / (uint32_t)(1024 * 1024),
        (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external",
        esp_get_minimum_free_heap_size() / (uint32_t)1024);
    ESP_LOGI(TAG, "port max delay %ld, port tick period ms %ld", portMAX_DELAY, portTICK_PERIOD_MS);

    /* initialize non-volatile storage, erasing if old */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    /* initialize the default event loop
     *   not necessary for a lot of basic applications, like arduino style stuff
     *   but things like the WiFi subsystem, etc. need this to exist
     */
    ESP_ERROR_CHECK(esp_event_loop_create_default());

#ifdef QWIIC_ENABLE_PIN
    ESP_LOGI(TAG, "enabling qwiic port using pin %d", QWIIC_ENABLE_PIN);
    gpio_config_t qwiic_conf = {
        .pin_bit_mask = (1ULL << QWIIC_ENABLE_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&qwiic_conf));
    ESP_ERROR_CHECK(gpio_set_level(QWIIC_ENABLE_PIN, 1));
    ESP_ERROR_CHECK(gpio_hold_en(QWIIC_ENABLE_PIN));
#endif

#ifdef ENABLE_SLEEP
    esp_pm_config_t pm_config;
    ESP_ERROR_CHECK(esp_pm_get_configuration(&pm_config));
    pm_config.light_sleep_enable = true;
    ESP_ERROR_CHECK(esp_pm_configure(&pm_config));
#endif
} // }}}

void app_main(void)
{ // {{{
    /* add '-D START_DELAY_MS=(5000)' to a build_flags setting in platformio.ini
     * to delay startup for 5s (or however long you want), useful if you're
     * trying to see serial output when resetting.
     */
#ifdef START_DELAY_MS
    ESP_LOGI(TAG, "delaying startup for %d ms", START_DELAY_MS);
    vTaskDelay(START_DELAY_MS / portTICK_PERIOD_MS);
#endif

    setup_chip();

    ESP_LOGI(TAG, "forcing twai silence on pin %d", TWAI_SLNT_PIN);
    gpio_config_t twai_slnt_conf = {
        .pin_bit_mask = (1ULL << TWAI_SLNT_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&twai_slnt_conf));
    ESP_ERROR_CHECK(gpio_set_level(TWAI_SLNT_PIN, 1));
    ESP_ERROR_CHECK(gpio_hold_en(TWAI_SLNT_PIN));

    ESP_LOGI(TAG, "can_message_t %d bytes, message buff %d bytes", sizeof(can_message_t),
             sizeof(MESSAGE_BUF));

    // TWAI_MODE_NORMAL or TWAI_MODE_NO_ACK to enable TX
    twai_general_config_t g_config =
        TWAI_GENERAL_CONFIG_DEFAULT(TWAI_TX_PIN, TWAI_RX_PIN, TWAI_MODE_LISTEN_ONLY);
    twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
    twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

    ESP_LOGI(TAG, "installing twai driver on tx %d, rx %d", TWAI_TX_PIN, TWAI_RX_PIN);
    if (twai_driver_install(&g_config, &t_config, &f_config) != ESP_OK) {
        ESP_LOGE(TAG, "failed to install twai driver!");
        return;
    }
    ESP_LOGI(TAG, "starting twai driver");
    if (twai_start() != ESP_OK) {
        ESP_LOGE(TAG, "failed to start twai driver!");
        return;
    }

    ESP_LOGI(TAG, "monitoring for twai messages");
    can_message_t *can_message = &MESSAGE_BUF->messages[MESSAGE_BUF->count];
    twai_message_t message = {0};
    while (true) {
        if (twai_receive(&message, 500 / portTICK_PERIOD_MS) != ESP_OK) {
            if (MESSAGE_BUF->count > 0) {
                // TODO: actually send message buffer via BLE
                MESSAGE_BUF->count = 0;
            }
            continue;
        }
        can_message->ts = NOW_MS;
        can_message->id = (uint16_t)message.identifier;
        can_message->sz = (uint16_t)message.data_length_code;
        memcpy(&can_message->data, &message.data, CAN_MESSAGE_SIZE);
        ++MESSAGE_BUF->count;
        if (can_message->id == 49 && can_message->data[0] == 0x06) {
            ESP_LOGI(TAG, "id %03ld, %d bytes, %02X %02X %02X %02X %02X %02X %02X %02X",
                     message.identifier, message.data_length_code, message.data[0], message.data[1],
                     message.data[2], message.data[3], message.data[4], message.data[5],
                     message.data[6], message.data[7]);
        }
        if (MESSAGE_BUF->count >= MESSAGE_BUF_SIZE) {
            MESSAGE_BUF->count = 0;
            can_message = &MESSAGE_BUF->messages[MESSAGE_BUF->count];
        }
        can_message = &MESSAGE_BUF->messages[MESSAGE_BUF->count];
    }
} // }}}
