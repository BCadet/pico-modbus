#include "gpios.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "modbus/modbus.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>
#include "panduza/dio.h"

int main()
{
    modbusController_t controller = {0};
    modbusDevice_t device = {0};
    const uint32_t gpioMask = ~((1<<31) | (1<<30) | (1<<29) | (1 << 23) | (1 << 24)); // disable io 23 and 24
    const pza_dio_coil_t coilWriteMask = {
        .gpios.direction = gpioMask,
        .gpios.pulls = gpioMask,
        .gpios.values = gpioMask | (1<<31)
    };

    pza_dio_regs_t dio = {0};
    pza_dio_init(&dio);

    device.address = dio.id;
    modbusDevice_add_coilRegister(&device, dio.coils.reg, sizeof(dio.coils.reg), coilWriteMask.reg);
    modbusDevice_add_inputRegister(&device, dio.identifier.reg, sizeof(dio.identifier.reg));
    modbusDevice_add_discretInputRegister(&device, dio.inputs.reg, sizeof(dio.inputs.reg));
    device.hwCallback = NULL;

    stdio_usb_init();

    gpios_init(gpioMask);

    modbus_init(&controller);
    modbus_add_device(&controller, &device);
    modbus_register_platform(&controller, platform_modbus_read, platform_modbus_write);

    while (true)
    {
        modbus_run(&controller);
        dio.inputs.content.inputs = gpios_update(
            dio.coils.gpios.direction,
            dio.coils.gpios.pulls,
            dio.coils.gpios.values);
        if(dio.coils.reg[11] & (1<<7))
            reset_usb_boot(0, 1);
    }
    return 0;
}
