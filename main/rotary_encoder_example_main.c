#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define ROTARY_ENCODER_CLK GPIO_NUM_25  // CLK pin
#define ROTARY_ENCODER_DT  GPIO_NUM_26  // DT pin
#define ROTARY_ENCODER_SW  GPIO_NUM_27  // Optional: SW (Button) pin

#define TAG "ROTARY_ENCODER"

// Use volatile for variables shared between ISR and tasks
volatile int current_value = 0;
volatile int direction_flag = 0;  // 1 for clockwise, -1 for counterclockwise, 0 for no movement
volatile int last_clk_state = 0;
volatile uint32_t last_interrupt_time = 0;  // Debouncing time

void IRAM_ATTR rotary_encoder_isr_handler(void* arg) {
    // Debouncing: Ignore interrupts that are too close together
    uint32_t current_time = xTaskGetTickCountFromISR();  // Get current time in ticks
    if ((current_time - last_interrupt_time) < pdMS_TO_TICKS(5)) {
        return;  // Skip this interrupt
    }
    last_interrupt_time = current_time;  // Update the last interrupt time

    // Read current states of CLK and DT pins
    int clk_state = gpio_get_level(ROTARY_ENCODER_CLK);
    int dt_state = gpio_get_level(ROTARY_ENCODER_DT);

    // Determine direction of rotation
    if (clk_state != last_clk_state) {
        if (dt_state != clk_state) {
            direction_flag = 1;  // Clockwise
        } else {
            direction_flag = -1;  // Counter-clockwise
        }
    }

    // Update last known CLK state
    last_clk_state = clk_state;
}

void rotary_encoder_task(void *pvParameter) {
    // Configure the GPIO for rotary encoder CLK and DT pins
    gpio_set_direction(ROTARY_ENCODER_CLK, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ROTARY_ENCODER_CLK, GPIO_PULLUP_ONLY);

    gpio_set_direction(ROTARY_ENCODER_DT, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ROTARY_ENCODER_DT, GPIO_PULLUP_ONLY);

    // Attach interrupt on the rising or falling edge of the CLK pin
    gpio_set_intr_type(ROTARY_ENCODER_CLK, GPIO_INTR_ANYEDGE);
    gpio_install_isr_service(0);  // Enable ISR service
    gpio_isr_handler_add(ROTARY_ENCODER_CLK, rotary_encoder_isr_handler, NULL);

    // If you want to handle button presses, configure the SW pin
    gpio_set_direction(ROTARY_ENCODER_SW, GPIO_MODE_INPUT);
    gpio_set_pull_mode(ROTARY_ENCODER_SW, GPIO_PULLUP_ONLY);

    while (1) {
        if(current_value < -99 || current_value > 99) {
            current_value = 0;
        }
        // Check direction flag set by the ISR
        if (direction_flag != 0) {
            // Update current value based on the direction
            current_value += direction_flag;
            if (direction_flag == 1) {
                ESP_LOGI(TAG, "Turned Right: Value = %d", current_value);
            } else if (direction_flag == -1) {
                ESP_LOGI(TAG, "Turned Left: Value = %d", current_value);
            }
            direction_flag = 0;  // Reset the flag after processing
        }

        // Optional: Handle button press (SW pin)
        if (gpio_get_level(ROTARY_ENCODER_SW) == 0) {
            ESP_LOGI(TAG, "Button Pressed");
            vTaskDelay(pdMS_TO_TICKS(200));  // Debounce delay
        }

        // Add some delay to reduce task overhead
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void app_main(void) {
    // Start the rotary encoder task
    xTaskCreate(&rotary_encoder_task, "rotary_encoder_task", 2048, NULL, 5, NULL);
}
