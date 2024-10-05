#include "iot_button.h"
#include "esp_log.h"

static const char *TAG = "BUTTON_APP";

#define NUM_BUTTONS 3
#define CONFIG_BUTTON_LONG_PRESS_TIME_MS 1000  // 1 seconds for long press
#define CONFIG_BUTTON_SHORT_PRESS_TIME_MS 20  // 0.2 seconds for short press

// GPIO pins for the buttons
const uint8_t button_pins[NUM_BUTTONS] = {23, 21};

// Callback function for button presses
void button_press_cb(void *btn_handle, void *usr_data) {
    uint8_t btn_idx = (uint8_t)(intptr_t)usr_data;
    ESP_LOGI(TAG, "Button %d Short Pressed", btn_idx + 1);
}

// Callback function for long button presses
void button_long_press_cb(void *btn_handle, void *usr_data) {
    uint8_t btn_idx = (uint8_t)(intptr_t)usr_data;
    ESP_LOGI(TAG, "Button %d Long Pressed", btn_idx + 1);
}

void app_main() {
    button_handle_t btn_handles[NUM_BUTTONS];

    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        // Create button handle
        btn_handles[i] = iot_button_create(&(button_config_t){
            .type = BUTTON_TYPE_GPIO,
            .long_press_time = CONFIG_BUTTON_LONG_PRESS_TIME_MS,
            .short_press_time = CONFIG_BUTTON_SHORT_PRESS_TIME_MS,
            .gpio_button_config = {
                .gpio_num = button_pins[i],
                .active_level = 0,
            },
        });

        if (btn_handles[i] == NULL) {
            ESP_LOGE(TAG, "Button %d creation failed", i + 1);
            continue; // Proceed to create the next button
        }

        // Register the callback for short press
        if (iot_button_register_cb(btn_handles[i], BUTTON_SINGLE_CLICK, button_press_cb, (void *)(intptr_t)i) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register short press callback for Button %d", i + 1);
        }

        // Register the callback for long press
        if (iot_button_register_cb(btn_handles[i], BUTTON_LONG_PRESS_START, button_long_press_cb, (void *)(intptr_t)i) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register long press callback for Button %d", i + 1);
        }
    }
}
