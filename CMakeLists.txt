cmake_minimum_required(VERSION 3.13.1)

set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(BOARD alturia)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)

set(ZEPHYR_BOARD_FLASH_RUNNER jlink)
project(alturia_firmware)

target_include_directories(app PRIVATE lib/inih/)

target_sources(app PRIVATE lib/inih/ini.c
			   src/util.c
			   src/sysinit.c
			   src/alturia.c
                           src/beeper.c
                           src/main.c
			   src/configuration.c )

