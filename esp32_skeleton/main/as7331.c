// as7331.c - AS7331 sensor driver source (stub)
#include "as7331.h"
#include <stdio.h>
#include "driver/i2c.h"

#define AS7331_ADDR 0x74 // page 42: AS7331 sensor's I2C address- Can run 4 AS7331s on the same I2C bus with A1 and A0 pins defining two lowest-order bits (high or low) - Assuming both A1 and A0 tied to GND
#define RESET_REG_AS7331 0x00 // page 48: OSR is at 0x00
#define RESET_VALUE_AS7331 0x08 // page 49: Set SW_RES bit (bit 3) to 1
#define CONFIG_REG_AS7331 0x00 // page 48: OSR is at 0x00
#define CONFIG_VALUE_AS7331 0x02 // page 49: 010 sets operational state to configuration
#define MEASUREMENT_REG_AS7331 0x00 // page 48: OSR is at 0x00
#define MEASUREMENT_VALUE_AS7331 0x03 // page 49: 011 sets operational state to measurement
#define CREG3_AS7331 0x08 // page 48: config register for clock frequency
#define CREG3_CCLK_VALUE 0x00 // page 56: 00 sets internal clock frequency to 1.024 MHz
#define CREG1_AS7331 0x06 // page 48: config register for gain and time is at 0x06
#define CREG1_TIME_GAIN_VALUE_AS7331 0xB3 // page 51: set gain to 1x and time to 8ms
#define I2C_MASTER_SCL 22 // ESP32's SCL pin
#define I2C_MASTER_SDA 21 // ESP32's SDA pin

void as7331_init(void) {
    // 1. (Optional) Initialize I2C bus here if not done elsewhere
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA, 
        .scl_io_num = I2C_MASTER_SCL, 
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000, // 400 kHz
        .clk_flags = 0,
        };
        
    // 2. Reset the sensor
    uint8_t reset_cmd[2] = {RESET_REG_AS7331, RESET_VALUE_AS7331};
    i2c_master_write_to_device(I2C_NUM_0, AS7331_ADDR, reset_cmd, 2, 1000 / portTICK_PERIOD_MS);

    // 3. Configure integration time, gain, etc. (see AS7331 datasheet for details)
    // Example: Write to a configuration register (replace reg and value)
    // uint8_t config_cmd[2] = {0xXX, 0xXX}; // 0xXX = config reg, 0xXX = config value
    // i2c_master_write_to_device(I2C_NUM_0, AS7331_ADDR, config_cmd, 2, 1000 / portTICK_PERIOD_MS);
    uint8_t OSR_set_mode_config_cmd[2] = {CONFIG_REG_AS7331, CONFIG_VALUE_AS7331}; // set OSR to configuration state
    i2c_master_write_to_device(I2C_NUM_0, AS7331_ADDR, OSR_set_mode_config_cmd, 2, 1000 / portTICK_PERIOD_MS);
    uint8_t config_cmd[2] = {CREG1_AS7331, CREG1_TIME_GAIN_VALUE_AS7331}; // set time to 8ms and gain to 1x
    i2c_master_write_to_device(I2C_NUM_0, AS7331_ADDR, config_cmd, 2, 1000 / portTICK_PERIOD_MS);

    // Add more configuration as needed

    printf("AS7331 initialized (real hardware)!\n");
}

// INCLUDE THE REST OF THE FUNCTIONS BELOW
