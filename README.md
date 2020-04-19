# Alturia Firmware

This repository contains the firmware of the Alturia rocket flight computer.

## Requirements

- Zephyr-RTOS v2.2 or greater
- Zephyr-SDK
- Python 3 and sympy for code generation

## Getting started

```bash
git clone https://github.com/rckTom/alturia-firmware.git
cd alturia-firmware

git submodule init && git submodule update

west build
west flash
```

## Code generation

Algorithms and filters are prototyped in python with sympy. You can use the
cmake command `symbolic_codegen()` to generate c code from symbolic equations.

## License

This work is licenced under the GNU General Public License version 3
