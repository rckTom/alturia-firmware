cmake_minimum_required(VERSION 3.13.1)

set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(BOARD alturia_v1_2)
set(BOARD_FLASH_RUNNER jlink)
set(BOARD_DEBUG_RUNNER jlink)
set(DTS_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(ZEPHYR_EXTRA_MODULES ${CMAKE_CURRENT_LIST_DIR}/lib/zephyr-lua)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
#include($ENV{ZEPHYR_BASE}/cmake/app/boilerplate.cmake NO_POLICY_SCOPE)


project(alturia_firmware)
set_property(TARGET app PROPERTY C_STANDARD 11)
target_include_directories(app PRIVATE lib/inih/
				       lib/cmsis/CMSIS/DSP/Include
				       src/generated/
				       src/filter/
				       src/
				       src/configuration
				       src/execution_engine/
				       src/lua_execution_engine/)

function(cached_symbolic_codegen)
	cmake_parse_arguments(
		ARGS
		""
		"TARGET;SYMBOLIC_SCRIPT;SYSNAME"
		""
		${ARGN}
	)

	add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/design/${ARGS_SYSNAME}.pkl
			   COMMAND python ${ARGS_SYMBOLIC_SCRIPT} --outfile ${CMAKE_CURRENT_LIST_DIR}/design/${ARGS_SYSNAME}.pkl
			   DEPENDS ${ARGS_SYMBOLIC_SCRIPT}
			   COMMENT "Generate symbolic representation of ${ARGS_SYSNAME}r")

	add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_SYSNAME}.c
				  ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_SYSNAME}.h
			   COMMAND python ${CMAKE_CURRENT_LIST_DIR}/design/codegen.py
					--input ${CMAKE_CURRENT_LIST_DIR}/design/${ARGS_SYSNAME}.pkl
					--prefix ${ARGS_SYSNAME}
					--outdir ${CMAKE_CURRENT_LIST_DIR}/src/generated &&
					uncrustify --replace --no-backup -l C -c ${CMAKE_CURRENT_LIST_DIR}/.uncrustify.cfg ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_SYSNAME}.c >/dev/null 2>&1 &&
					uncrustify --replace --no-backup -l C -c ${CMAKE_CURRENT_LIST_DIR}/.uncrustify.cfg ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_SYSNAME}.h >/dev/null 2>&1
			   DEPENDS ${CMAKE_CURRENT_LIST_DIR}/design/${ARGS_SYSNAME}.pkl
			   COMMENT "Generate c code for ${ARGS_SYSNAME}")

	target_sources(${ARGS_TARGET} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_SYSNAME}.c
					      ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_SYSNAME}.h)
endfunction()

function(symbolic_codegen)
	cmake_parse_arguments(
		ARGS
		""
		"TARGET;SYMBOLIC_SCRIPT;PREFIX"
		""
		${ARGN}
	)

	add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_PREFIX}.c
				  ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_PREFIX}.h
				  COMMAND python ${ARGS_SYMBOLIC_SCRIPT}
				  --prefix ${ARGS_PREFIX}
				  --outdir ${CMAKE_CURRENT_LIST_DIR}/src/generated &&
				  uncrustify --replace --no-backup -l C -c ${CMAKE_CURRENT_LIST_DIR}/.uncrustify.cfg ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_PREFIX}.c >/dev/null 2>&1 &&
				  uncrustify --replace --no-backup -l C -c ${CMAKE_CURRENT_LIST_DIR}/.uncrustify.cfg ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_PREFIX}.h >/dev/null 2>&1
		     DEPENDS ${ARGS_SYMBOLIC_SCRIPT}
		     COMMENT "Generate c code for ${ARGS_PREFIX}")

	target_sources(${ARGS_TARGET} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_PREFIX}.c
					      ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_PREFIX}.h)
endfunction()

cached_symbolic_codegen(TARGET app
	SYMBOLIC_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/design/altitude_kalman_filter.py
	SYSNAME altitude_kalman_impl)

cached_symbolic_codegen(TARGET app
	SYMBOLIC_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/design/vertical_dynamics_kalman_filter.py
	SYSNAME vertical_dynamics_kalman_impl)

cached_symbolic_codegen(TARGET app
	SYMBOLIC_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/design/constant_altitude_kalman_filter.py
	SYSNAME constant_altitude_kalman_impl)

symbolic_codegen(TARGET app
	SYMBOLIC_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/design/transformations.py
	PREFIX transformations_impl)

symbolic_codegen(TARGET app
	SYMBOLIC_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/design/attitude_estimation.py
	PREFIX attitude_estimation_impl)

symbolic_codegen(TARGET app
	SYMBOLIC_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/design/mahony_filter.py
	PREFIX mahohny_attitude_estimation_impl)

zephyr_linker_sources(DATA_SECTIONS iterables.ld)

target_sources(app PRIVATE lib/inih/ini.c
			   src/util.c
			   src/signals.c
			   src/sysinit.c
			   src/alturia.c
			   src/beeper.c
			   #src/pil.c
			   src/main.c
			   src/led.c
			   src/events2.c
			   src/datalogger.c
			   src/is_atmosphere.c
			   src/pyros.c
			   src/daq.c
			   src/daq_sensor.c
			   src/generic.c
			   src/filter/kalman_filter.c
			   src/filter/attitude_estimation.c
			   src/math_ex.c
			   src/servos.c
			   src/eventtimer.c
			   src/edge_detector.c
			   src/lua_execution_engine/lua_execution_engine.c
			   src/lua_execution_engine/lua_ledlib.c
			   src/lua_execution_engine/lua_timerlib.c
			   src/lua_execution_engine/lua_servolib.c
			   src/lua_execution_engine/lua_signallib.c
			   src/lua_execution_engine/lua_eventlib.c
			   src/lua_execution_engine/lua_pyrolib.c
			   src/rocket/signal_processing.c
			   src/rocket/flightstate.c
			   src/rocket/data_logging.c
			   src/pwm_rgb_led.c
			   src/filter/pt1.c)

