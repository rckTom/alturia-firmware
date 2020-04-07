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
				       src/generated/
				       src/filter/)

function(generate_linear_kalman_filter)
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
			   COMMENT "Generate symbolic representation of ${ARGS_SYSNAME} kalman filter")

	add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_SYSNAME}.c
				  ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_SYSNAME}.h
			   COMMAND python ${CMAKE_CURRENT_LIST_DIR}/design/gen_code_linear_kalman_filter.py
					--input ${CMAKE_CURRENT_LIST_DIR}/design/${ARGS_SYSNAME}.pkl
					--prefix ${ARGS_SYSNAME}
					--outdir ${CMAKE_CURRENT_LIST_DIR}/src/generated &&
					uncrustify --replace --no-backup -l C -c ${CMAKE_CURRENT_LIST_DIR}/.uncrustify.cfg ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_SYSNAME}.c >/dev/null 2>&1 &&
					uncrustify --replace --no-backup -l C -c ${CMAKE_CURRENT_LIST_DIR}/.uncrustify.cfg ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_SYSNAME}.h >/dev/null 2>&1
			   DEPENDS ${CMAKE_CURRENT_LIST_DIR}/design/${ARGS_SYSNAME}.pkl
			   COMMENT "Generate c code for ${ARGS_SYSNAME} kalman filter")

	target_sources(${ARGS_TARGET} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_SYSNAME}.c
					      ${CMAKE_CURRENT_LIST_DIR}/src/generated/${ARGS_SYSNAME}.h)
endfunction()

generate_linear_kalman_filter(TARGET app
			      SYMBOLIC_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/design/altitude_kalman_filter.py
			      SYSNAME altitude_kalman_impl)

generate_linear_kalman_filter(TARGET app
			      SYMBOLIC_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/design/vertical_dynamics_kalman_filter.py
			      SYSNAME vertical_dynamics_kalman_impl)

generate_linear_kalman_filter(TARGET app
			      SYMBOLIC_SCRIPT ${CMAKE_CURRENT_LIST_DIR}/design/constant_altitude_kalman_filter.py
			      SYSNAME constant_altitude_kalman_impl)

target_sources(app PRIVATE lib/inih/ini.c
			   src/util.c
			   src/sysinit.c
			   src/alturia.c
                           src/beeper.c
                           src/main.c
			   src/configuration.c
			   src/datalogger.c
			   src/is_atmosphere.c
			   src/pyros.c
			   src/daq.c
			   src/filter/kalman_filter.c)

target_link_libraries(app PRIVATE ${CMAKE_CURRENT_LIST_DIR}/lib/cmsis/CMSIS/DSP/Lib/GCC/libarm_cortexM4lf_math.a)
