set(srcs
    "console.c"
    "display/disp_app.c"
    "display/disp_demos.c"
    "display/disp_indev.c"
    "display/disp_setup.c"
    "display/disp_window_datalog.c"
    "display/disp_window_graph.c"
    "display/disp_window_menu.c"
    "display/disp_window_view.c"
    "main.c"
    "usb_composite.c"
    #"encoder.c"
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
