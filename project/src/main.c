#include "gpios.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "modbus/modbus.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include "panduza.h"

int main()
{
    struct modbusController controller = {0};
    struct modbusDevice device = {0};
    pza_gpio_t coil = {0};
    const uint32_t gpioMask = ~((1<<31) | (1<<30) | (1 << 23) | (1 << 24)); // disable io 23 and 24
    const pza_gpio_t coilWriteMask = {
        .gpios.direction = gpioMask,
        .gpios.pulls = gpioMask,
        .gpios.values = gpioMask | (1<<31)
    };

    device.address = 0x01;
    modbusDevice_add_coilRegister(&device, coil.reg, sizeof(coil.reg), coilWriteMask.reg);
    // modbusDevice_add_holdingRegister(&device, holding, sizeof(holding), NULL);
    device.hwCallback = NULL;

    stdio_usb_init();

    gpios_init(gpioMask);

    modbus_init(&controller);
    modbus_add_device(&controller, &device);
    modbus_register_platform(&controller, platform_modbus_read, platform_modbus_write);

    while (true)
    {
        modbus_run(&controller);
        gpios_update(
            coil.gpios.direction,
            coil.gpios.pulls,
            &coil.gpios.values);
        if(coil.reg[11] & (1<<7))
            reset_usb_boot(0, 1);
    }
    return 0;
}
