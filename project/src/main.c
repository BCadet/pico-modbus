#include "modbus/modbus.h"
#include "connecteurs/modbus.h"
#include <stdio.h>
#include <string.h>
#include "panduza/dio.h"

int main()
{
    modbusController_t controller = {0};
    modbusDevice_t dio_device = {0};
    const uint32_t gpioMask = ~((1<<31) | (1<<30) | (1<<29) | (1 << 23) | (1 << 24)); // disable io 23 and 24
    const pza_dio_control_t coilWriteMask = {
        .gpios.direction = gpioMask,
        .gpios.pulls = gpioMask,
        .gpios.values = gpioMask | (1<<31)
    };

    pza_dio_regs_t dio = {0};

    panduza_platform_init();

    dio_device.address = dio.id;
    modbusDevice_add_coilRegister(&dio_device, dio.control.reg, sizeof(dio.control.reg), coilWriteMask.reg);
    modbusDevice_add_inputRegister(&dio_device, dio.identifier.reg, sizeof(dio.identifier.reg));
    modbusDevice_add_discretInputRegister(&dio_device, dio.inputs.reg, sizeof(dio.inputs.reg));
    dio_device.hwCallback = NULL;

    pza_dio_init(&dio, gpioMask);

    modbus_init(&controller);
    modbus_add_device(&controller, &dio_device);
    modbus_register_platform(&controller, platform_modbus_read, platform_modbus_write);

    while (1)
    {
        modbus_run(&controller);
        pza_dio_run(&dio);
        // if(dio.control.reg[11] & (1<<7))
        //     reset_usb_boot(0, 1);
    }
    return 0;
}
