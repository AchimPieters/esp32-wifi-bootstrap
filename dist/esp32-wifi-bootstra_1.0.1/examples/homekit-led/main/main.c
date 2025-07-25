/**
   Copyright 2025 Achim Pieters | StudioPieters®

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in all
   copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NON INFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
   WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   for more information visit https://www.studiopieters.nl
 **/

#include <stdio.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <wifi_config.h>

// GPIO-definities
#define LED_GPIO CONFIG_ESP_LED_GPIO
#define BUTTON_GPIO CONFIG_ESP_RESET_GPIO
#define DEBOUNCE_TIME_MS 50

static const char *TAG = "main";

bool led_on = false;

void led_write(bool on) {
        gpio_set_level(LED_GPIO, on ? 1 : 0);
}

void gpio_init() {
        // LED setup
        gpio_reset_pin(LED_GPIO);
        gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
        led_write(led_on);

        // Knop setup
        gpio_config_t io_conf = {
                .pin_bit_mask = 1ULL << BUTTON_GPIO,
                        .mode = GPIO_MODE_INPUT,
                        .pull_up_en = GPIO_PULLUP_ENABLE,
                        .pull_down_en = GPIO_PULLDOWN_DISABLE,
                        .intr_type = GPIO_INTR_DISABLE
        };
        gpio_config(&io_conf);
}

// Task factory_reset
void factory_reset_task(void *pvParameter) {
        ESP_LOGI("RESET", "Resetting WiFi Config");
        wifi_config_reset();

        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_LOGI("RESET", "Resetting HomeKit Config");
        homekit_server_reset();

        vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_LOGI("RESTART", "Restarting system");
        esp_restart();

        vTaskDelete(NULL);
}

void factory_reset() {
        ESP_LOGI("RESET", "Resetting device configuration");
        xTaskCreate(factory_reset_task, "factory_reset", 4096, NULL, 2, NULL);
}

// Task button
void button_task(void *pvParameter) {
        ESP_LOGI(TAG, "Button task started");
        bool last_state = true;

        while (1) {
                bool current_state = gpio_get_level(BUTTON_GPIO);

                // Detecteer overgang van HIGH naar LOW (knop ingedrukt)
                if (last_state && !current_state) {
                        ESP_LOGW(TAG, "BUTTON PRESSED → RESETTING CONFIGURATION");
                        factory_reset();
                }

                last_state = current_state;
                vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME_MS));
        }
}

// Accessory identify
void accessory_identify_task(void *args) {
        for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 2; j++) {
                        led_write(true);
                        vTaskDelay(pdMS_TO_TICKS(100));
                        led_write(false);
                        vTaskDelay(pdMS_TO_TICKS(100));
                }
                vTaskDelay(pdMS_TO_TICKS(250));
        }
        led_write(led_on);
        vTaskDelete(NULL);
}

void accessory_identify(homekit_value_t _value) {
        ESP_LOGI(TAG, "Accessory identify");
        xTaskCreate(accessory_identify_task, "identify", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}

// HomeKit callbacks
homekit_value_t led_on_get() {
        return HOMEKIT_BOOL(led_on);
}

void led_on_set(homekit_value_t value) {
        if (value.format != homekit_format_bool) {
                ESP_LOGE(TAG, "Invalid value format: %d", value.format);
                return;
        }
        led_on = value.bool_value;
        led_write(led_on);
}

// HomeKit metadata
#define DEVICE_NAME "LED"
#define DEVICE_MANUFACTURER "StudioPieters®"
#define DEVICE_SERIAL "NLDA4SQN1466"
#define DEVICE_MODEL "SD466NL/A"
#define FW_VERSION "0.0.1"

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, DEVICE_NAME);
homekit_characteristic_t manufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER, DEVICE_MANUFACTURER);
homekit_characteristic_t serial = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, DEVICE_SERIAL);
homekit_characteristic_t model = HOMEKIT_CHARACTERISTIC_(MODEL, DEVICE_MODEL);
homekit_characteristic_t revision = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION, FW_VERSION);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"
homekit_accessory_t *accessories[] = {
        HOMEKIT_ACCESSORY(.id = 1, .category = homekit_accessory_category_lighting, .services = (homekit_service_t*[]) {
                HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics = (homekit_characteristic_t*[]) {
                        &name,
                        &manufacturer,
                        &serial,
                        &model,
                        &revision,
                        HOMEKIT_CHARACTERISTIC(IDENTIFY, accessory_identify),
                        NULL
                }),
                HOMEKIT_SERVICE(LIGHTBULB, .primary = true, .characteristics = (homekit_characteristic_t*[]) {
                        HOMEKIT_CHARACTERISTIC(NAME, DEVICE_NAME),
                        HOMEKIT_CHARACTERISTIC(ON, false, .getter = led_on_get, .setter = led_on_set),
                        NULL
                }),
                NULL
        }),
        NULL
};
#pragma GCC diagnostic pop

homekit_server_config_t config = {
        .accessories = accessories,
        .password = CONFIG_ESP_SETUP_CODE,
        .setupId = CONFIG_ESP_SETUP_ID,
};

void on_wifi_ready() {
        ESP_LOGI(TAG, "WiFi ready, starting HomeKit");
        homekit_server_init(&config);
}

void app_main(void) {
        ESP_ERROR_CHECK(nvs_flash_init());
        gpio_init();
        xTaskCreate(button_task, "button_task", 2048, NULL, 10, NULL);
        wifi_config_init(DEVICE_NAME, NULL, on_wifi_ready);
}
