#include <stdio.h>
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#define OUTPUT_ENABLE  GPIO_NUM_18  // GPIO18 - Push Button 1
#define OUTPUT_SELECT  GPIO_NUM_19  // GPIO19 - Push Button 2
#define LED_DATA       GPIO_NUM_23  // GPIO23 - Debug LED for DATA
#define LED_SYNC       GPIO_NUM_5   // GPIO5 - Debug LED for SYNC

// Define timing parameters
#define TON1  1200  // First pulse on-time (us)
#define TOFF  600   // Pulse off-time (us)
#define PULSES 13   // Number of pulses
#define IDLE  6000  // Idle time before SYNC (us)
#define SYNC_ON 50  // SYNC pulse on-time (us)

// Variables to track state
bool dataEnabled = false;
bool reversedMode = false;

// Button state tracking
bool lastEnableButtonState = true;
bool lastSelectButtonState = true;
uint64_t lastDebounceTimeEnable = 0;
uint64_t lastDebounceTimeSelect = 0;
const uint64_t debounceDelay = 200000; // 200ms debounce time in microseconds

uint64_t get_time_us()
 {
    return esp_timer_get_time();
}

void check_buttons() 
{
    bool enableButtonState = gpio_get_level(OUTPUT_ENABLE) == 0;
    bool selectButtonState = gpio_get_level(OUTPUT_SELECT) == 0;
    uint64_t currentTime = get_time_us();

    if (enableButtonState && !lastEnableButtonState && (currentTime - lastDebounceTimeEnable > debounceDelay)) 
    {
        dataEnabled = !dataEnabled;
        ESP_LOGI("BUTTON", "Toggling dataEnabled to: %s", dataEnabled ? "true" : "false");
        gpio_set_level(LED_DATA, dataEnabled);
        lastDebounceTimeEnable = currentTime;
    }
    lastEnableButtonState = enableButtonState;

    if (selectButtonState && !lastSelectButtonState && (currentTime - lastDebounceTimeSelect > debounceDelay))
     {
        reversedMode = !reversedMode;
        ESP_LOGI("BUTTON", "Toggling reversedMode to: %s", reversedMode ? "true" : "false");
        gpio_set_level(LED_SYNC, reversedMode);
        lastDebounceTimeSelect = currentTime;
    }
    lastSelectButtonState = selectButtonState;
}

void generate_waveform() 
{
    ESP_LOGI("WAVEFORM", "Generating %s waveform.", reversedMode ? "reversed" : "normal");

    if (!reversedMode) 
    {
        for (int n = 1; n <= PULSES; n++) 
        {
            ESP_LOGI("WAVEFORM", "Pulse %d: LED_DATA HIGH for %dus", n, TON1 + (n - 1) * 50);
            gpio_set_level(LED_DATA, 1);
            esp_timer_delay_us(TON1 + (n - 1) * 50);
            gpio_set_level(LED_DATA, 0);
            esp_timer_delay_us(TOFF);
        }
    } else
     {
        for (int n = PULSES; n >= 1; n--) 
        {
            ESP_LOGI("WAVEFORM", "Pulse %d: LED_DATA HIGH for %dus", n, TON1 + (n - 1) * 50);
            gpio_set_level(LED_DATA, 1);
            esp_timer_delay_us(TON1 + (n - 1) * 50);
            gpio_set_level(LED_DATA, 0);
            esp_timer_delay_us(TOFF);
        }
    }

    ESP_LOGI("WAVEFORM", "Idle for %dus", IDLE);
    esp_timer_delay_us(IDLE);

    ESP_LOGI("WAVEFORM", "Generating SYNC pulse...");
    gpio_set_level(LED_SYNC, 1);
    esp_timer_delay_us(SYNC_ON);
    gpio_set_level(LED_SYNC, 0);
}

void app_main() {
    gpio_config_t io_conf = 
    {
        .pin_bit_mask = (1ULL << OUTPUT_ENABLE) | (1ULL << OUTPUT_SELECT),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    io_conf.pin_bit_mask = (1ULL << LED_DATA) | (1ULL << LED_SYNC);
    io_conf.mode = GPIO_MODE_OUTPUT;
    gpio_config(&io_conf);

    while (1) {
        check_buttons();
        if (dataEnabled) 
        {
            generate_waveform();
        }
    }
}
