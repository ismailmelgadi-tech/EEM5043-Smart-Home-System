idf_component_register(
    SRCS 
        "app_main.c" 
        "app_driver.c" 
        "ssd1306.c" 
        "dht11.c"
    INCLUDE_DIRS "." 
    REQUIRES 
        esp_rainmaker 
        freertos 
        driver 
        esp_system 
        nvs_flash 
        log
        esp_event
        rmaker_app_network 
        # تم إزالة esp_insights
)