cmake_minimum_required(VERSION 3.13.1)

set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(BOARD alturia)
set(BOARD_FLASH_RUNNER jlink)
set(BOARD_DEBUG_RUNNER jlink)
set(DTS_ROOT ${CMAKE_CURRENT_LIST_DIR})

include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)


project(alturia_firmware)
set_property(TARGET app PROPERTY C_STANDARD 11)
target_include_directories(app PRIVATE lib/inih/
				       lib/cmsis/CMSIS/DSP/Include
				       src/generated/)

add_custom_command(OUTPUT  ${CMAKE_CURRENT_LIST_DIR}/design/vertical_dynamics_kalman_filter.pkl
		   WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/design
		   COMMAND python ${CMAKE_CURRENT_LIST_DIR}/design/vertical_dynamics_kalman_filter.py
		   DEPENDS ${CMAKE_CURRENT_LIST_DIR}/design/vertical_dynamics_kalman_filter.py)

add_custom_command(OUTPUT  ${CMAKE_CURRENT_LIST_DIR}/src/generated/kalman_vertical_dynamics.c
			   ${CMAKE_CURRENT_LIST_DIR}/src/generated/kalman_vertical_dynamics.h
	           WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/design
		   COMMAND python ${CMAKE_CURRENT_LIST_DIR}/design/gen_code.py
		   DEPENDS ${CMAKE_CURRENT_LIST_DIR}/design/vertical_dynamics_kalman_filter.pkl
		   	   ${CMAKE_CURRENT_LIST_DIR}/design/vertical_dynamics_kalman_filter.py
		   COMMENT "Create C code for vertical dynamics kalman filter")

target_sources(app PRIVATE lib/inih/ini.c
			   src/util.c
			   src/sysinit.c
			   src/alturia.c
                           src/beeper.c
                           src/main.c
			   src/configuration.c
			   src/datalogger.c
			   src/is_atmosphere.c
			   src/kalman.c
			   src/pyros.c
			   src/generated/kalman_vertical_dynamics.c)

target_link_libraries(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/cmsis/CMSIS/DSP/Lib/GCC/libarm_cortexM4lf_math.a)
