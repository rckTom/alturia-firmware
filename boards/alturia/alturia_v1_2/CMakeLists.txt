if(CONFIG_SPI_NOR)
zephyr_library()
zephyr_library_sources(flash_hold_pin_setup.c)
endif()
