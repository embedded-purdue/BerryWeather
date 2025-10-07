idf_component_register(SRCS "modular.c" "main.c"
                       PRIV_REQUIRES spi_flash esp_driver_gpio esp_driver_uart
                       INCLUDE_DIRS "")
