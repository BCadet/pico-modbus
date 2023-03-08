// Include implementation here
#define LIGHTMODBUS_IMPL
// Library configuration
#define LIGHTMODBUS_SLAVE_FULL
#define LIGHTMODBUS_DEBUG
#include "lightmodbus.h"

#include "platform.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "modbus.h"


static ModbusSlave slave;

struct registerMap map = {0};

ModbusError registerCallback(
    const ModbusSlave *slave,
    const ModbusRegisterCallbackArgs *args,
    ModbusRegisterCallbackResult *result)
{
    printf("\r\nAccess query: %s at index %d", modbusRegisterQueryStr(args->query), args->index);

    switch (args->query)
    {
    // Pretend to allow all access
    case MODBUS_REGQ_R_CHECK:
    case MODBUS_REGQ_W_CHECK:
        if(map.WP[args->index])
            result->exceptionCode = MODBUS_EXCEP_ILLEGAL_VALUE;
        else
            result->exceptionCode = MODBUS_EXCEP_NONE;
        break;

    case MODBUS_REGQ_R:
        result->value = map.registers[args->index];
        break;

    case MODBUS_REGQ_W:
        map.registers[args->index] = args->value;
    default:
        break;
    }

    return MODBUS_OK;
}

ModbusError exceptionCallback(const ModbusSlave *slave, uint8_t function, ModbusExceptionCode code)
{
    printf("\r\nSlave exception %s (function %d)", modbusExceptionCodeStr(code), function);
    return MODBUS_OK;
}

void modbus_init(void)
{
    // Create a slave
    ModbusErrorInfo err = modbusSlaveInit(
                                &slave,
                                registerCallback,
                                exceptionCallback,
                                modbusDefaultAllocator,
                                modbusSlaveDefaultFunctions,
                                modbusSlaveDefaultFunctionCount);

}

ModbusSlave* get_modbus_slave(void)
{
    return &slave;
}