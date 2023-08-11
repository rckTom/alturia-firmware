# Alturia Firmware
[![Compiles](https://github.com/rckTom/alturia-firmware/actions/workflows/build.yml/badge.svg)](https://github.com/rckTom/alturia-firmware/actions/workflows/build.yml)

This repository contains the firmware of the Alturia rocket flight computer.

## Requirements

- Zephyr-RTOS
	- Currently only builds with the custom fork on https://github.com/rcktom/zephyr because the imu driver is not upstreamed yet.
- Zephyr-SDK
- Following python packages:
	* sympy
- uncrustify

## Getting started

```bash
git clone https://github.com/rckTom/alturia-firmware.git
cd alturia-firmware

git submodule init && git submodule update

west build -b alturia_v1_2
west flash
```

## Code generation

Algorithms and filters are prototyped in python with sympy. You can use the
cmake command `symbolic_codegen()` to generate c code from symbolic equations.

## SIL Testing

For the alturia_v1_2 board there is sil testing capabilites

## License

This work is licenced under the GNU General Public License version 3
