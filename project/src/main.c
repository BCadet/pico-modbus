#include "gpios.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "modbus.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>

int main()
{
    struct modbusController controller = {0};
    struct modbusDevice device = {0};
    uint8_t data[8] = {0};
    const uint32_t gpioMask = ~((1<<31)|(1<<30)|(1 << 23) | (1 << 24)); // disable io 23 and 24
    uint8_t writeMask[8];
    *(uint32_t*)(writeMask) = gpioMask;
    *(uint32_t*)(writeMask+4) = gpioMask;

    device.accessTypeMask = MODBUS_COIL | MODBUS_DISCRETE_INPUT;
    device.address = 0x01;
    device.data.u8 = data;
    device.dataLen = sizeof(data);
    device.writableMask = writeMask;

    stdio_usb_init();

    gpios_init(gpioMask);

    modbus_init(&controller);
    modbus_add_device(&controller, &device);
    modbus_register_platform(&controller, platform_modbus_read, platform_modbus_write);

    while (true)
    {
        modbus_run(&controller);
        gpios_update(
            (uint32_t)(*(uint32_t *)data),
            (uint32_t *)(data + 4));
    }
    return 0;
}
