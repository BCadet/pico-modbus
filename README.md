# pico-modbus

A modbus slave device based on the raspberry pi pico in C

## How to build

```bash
mkdir build
cd build
cmake ../project
make
```

## Debug

This environment allow to debug the pico code with if a picoprobe is connected and openocd at least version 0.12.0 is installed. THe devcontainer is already configured to perform debug.

