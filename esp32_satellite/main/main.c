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



#define SATELLITE_DEVICE
// #define MM_DEVICE

#ifdef SATELLITE_DEVICE
    // Function to send LoRa message with AT+SEND command
    void send_lora_message(uint8_t address, const char* message) {
        char at_command[256];  // Buffer for the complete AT command
        int message_length = strlen(message);
        
        // Format: AT+SEND=address,length,message
        snprintf(at_command, sizeof(at_command), "AT+SEND=%d,%d,%s\r\n", 
                address, message_length, message);
        
        // Send the AT command
        uart_write_bytes(RYLR998_UART_PORT, at_command, strlen(at_command));
        printf("Sent: %s", at_command);
    }

    // Task to send a test message
    void send_test(void *arg)
    {
        vTaskDelay(pdMS_TO_TICKS(5000));
        const char *message = "Hello from ESP32 via RYLR998!"; 

        // Send message (address 1 is MM device)
        send_lora_message(1, message);
        vTaskDelete(NULL);
    }


    // Task to send AT commands for setup
    void lora_satellite_setup(void *arg)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        const char *commands[] = {
            "AT\r\n",
            "AT+VER?\r\n",
            "AT+UID?\r\n",
            "AT+MODE=0\r\n",
            "AT+ADDRESS=0\r\n",
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

        // Start task to send test message
        xTaskCreate(send_test, "send_test", 2048, NULL, 5, NULL);
        vTaskDelete(NULL);
    }
#endif

#ifdef MM_DEVICE
    void lora_mm_setup(void *arg)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
        const char *commands[] = {
            "AT\r\n",
            "AT+VER?\r\n",
            "AT+UID?\r\n",
            "AT+MODE=0\r\n",
            "AT+ADDRESS=1\r\n",
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

        // listen for incoming messages
        while (1) {
            uint8_t data[BUF_SIZE];
            int len = uart_read_bytes(RYLR998_UART_PORT, data, BUF_SIZE - 1, pdMS_TO_TICKS(1000));
            if (len > 0) {
                data[len] = '\0';
                printf("Received: %s\n", data);
            }
        }
    }
#endif

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
    };
    gpio_config(&io_conf);

    // RYLR998 reset
    gpio_set_level(RYLR998_NRST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(RYLR998_NRST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    uart_flush_input(RYLR998_UART_PORT);


    #ifdef MM_DEVICE
        xTaskCreate(lora_mm_setup, "lora_mm_setup", 4096, NULL, 5, NULL);
    #endif

    #ifdef SATELLITE_DEVICE
        xTaskCreate(lora_satellite_setup, "lora_satellite_setup", 4096, NULL, 5, NULL);
    #endif
}