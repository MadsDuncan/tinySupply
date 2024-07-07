set(srcs 
    "console.c"
    "encoder.c"
    "main.c"
    "usb_composite.c"
    "display/display.c"
)

set(include_dirs 
    "."
    "display"
)

set(priv_requires
    "console"
    "esp_partition"
    "esp_timer"
    "fatfs"
    "lvgl"
    "lvgl_esp32_drivers"
    "wear_levelling"
)

idf_component_register(
    SRCS "${srcs}"
    INCLUDE_DIRS "${include_dirs}"
    PRIV_REQUIRES "${priv_requires}"
)
