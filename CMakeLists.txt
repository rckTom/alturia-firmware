cmake_minimum_required(VERSION 3.13.1)

set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(BOARD alturia)

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)

project(alturia_firmware)

target_sources(app PRIVATE src/main.c)
