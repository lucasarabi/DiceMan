#include "iot_button.h"
#include "esp_log.h"

static const char *TAG = "BUTTON_APP";

#define NUM_BUTTONS 3

// GPIO pins for the buttons
//23 are normal buttons
//21 is petentiometer button
const uint8_t button_pins[NUM_BUTTONS] = {23, 21};

// Callback function for button presses
void button_press_cb(void *btn_handle, void *usr_data) {
    uint8_t btn_idx = (uint8_t)(intptr_t)usr_data;
    ESP_LOGI(TAG, "Button %d Pressed", btn_idx + 1);
}

void app_main() {
    button_handle_t btn_handles[NUM_BUTTONS];

    for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
        // Create button handle
        btn_handles[i] = iot_button_create(&(button_config_t){
            .type = BUTTON_TYPE_GPIO,
            .gpio_button_config = {
                .gpio_num = button_pins[i],
                .active_level = 0,
            },
        });

        if (btn_handles[i] == NULL) {
            ESP_LOGE(TAG, "Button %d creation failed", i + 1);
            continue; // Proceed to create the next button
        }

        // Register the callback, passing the button index as user data
        if (iot_button_register_cb(btn_handles[i], BUTTON_SINGLE_CLICK, button_press_cb, (void *)(intptr_t)i) != ESP_OK) {
            ESP_LOGE(TAG, "Failed to register callback for Button %d", i + 1);
        }
    }
}