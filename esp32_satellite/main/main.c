#include <stdio.h>
#include <string.h>
#include "driver/uart.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define RYLR998_UART_PORT    UART_NUM_1
#define RYLR998_TXD_PIN      GPIO_NUM_17   // ESP32 TX → module RX
#define RYLR998_RXD_PIN      GPIO_NUM_16   // ESP32 RX ← module TX
#define RYLR998_NRST_PIN     GPIO_NUM_4    // (active low) reset pin
#define BUF_SIZE        1024

void at_command_task(void *arg)
{
    vTaskDelay(pdMS_TO_TICKS(500));
    const char *commands[] = {
        "AT\r\n",
        "AT+VER?\r\n",
        "AT+UID?\r\n"
    };
    const int num_commands = sizeof(commands) / sizeof(commands[0]);

    for (int i = 0; i < num_commands; i++) {
        // Send command
        uart_write_bytes(RYLR998_UART_PORT, commands[i], strlen(commands[i]));
        printf("Sent: %s", commands[i]);

        // Wait for response
        uint8_t data[BUF_SIZE];
        int len = uart_read_bytes(RYLR998_UART_PORT, data, BUF_SIZE - 1, pdMS_TO_TICKS(1000));
        if (len > 0) {
            data[len] = '\0';
            printf("Received: %s\n", data);
        }
    }

    vTaskDelete(NULL);
}

void app_main(void)
{
    // Configure UART
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    uart_param_config(RYLR998_UART_PORT, &uart_config);
    uart_set_pin(RYLR998_UART_PORT, RYLR998_TXD_PIN, RYLR998_RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(RYLR998_UART_PORT, BUF_SIZE * 2, 0, 0, NULL, 0);

    // Configure the NRST pin as output with pull-up enabled
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RYLR998_NRST_PIN),
        .mode = GPIO_MODE_OUTPUT_OD,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    // RYLR998 reset
    gpio_set_level(RYLR998_NRST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(RYLR998_NRST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    uart_flush_input(RYLR998_UART_PORT);


    // Tasks
    xTaskCreate(at_command_task, "at_command_task", 4096, NULL, 5, NULL);
}