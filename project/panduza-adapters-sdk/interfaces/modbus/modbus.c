// Include implementation here
#define LIGHTMODBUS_IMPL
// Library configuration
#define LIGHTMODBUS_SLAVE_FULL
#define LIGHTMODBUS_DEBUG
#include "lightmodbus/lightmodbus.h"

#include "modbus/modbus.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ModbusError modbusStaticAllocator(ModbusBuffer *buffer, uint16_t size, void *context)
{
    static uint8_t static_buf[MODBUS_RTU_ADU_MAX];
    static uint8_t allocated = 0;

    ModbusError retCode = MODBUS_OK;

    if (size != 0) // allocation request
    {
        if (size <= MODBUS_RTU_ADU_MAX && allocated == 0) // Allocation request is within bounds
        {
            buffer->data = (uint8_t *)&static_buf;
            allocated = 1;
        }
        else // Allocation error
        {
            buffer->data = NULL;
            retCode = MODBUS_ERROR_ALLOC;
        }
    }
    else // Free request
    {
        allocated = 0;
        buffer->data = NULL;
    }

    return retCode;
}

ModbusError registerCallback(
    const ModbusSlave *slave,
    const ModbusRegisterCallbackArgs *args,
    ModbusRegisterCallbackResult *result)
{
    modbusDevice_t *device = slave->context;
    modbus_log("\r\n%.2x | %-23s | %-19s | %0.3d | ", device->address, modbusDataTypeStr(args->type), modbusRegisterQueryStr(args->query), args->index);

    result->exceptionCode = MODBUS_EXCEP_NONE;

    modbusRegister_t *reg = NULL;

    // get the corresponding register
    switch(args->type)
    {
        case MODBUS_HOLDING_REGISTER:
            reg = &device->registers.holding;
            break;
        case MODBUS_INPUT_REGISTER:
            reg = &device->registers.input;
            break;
        case MODBUS_COIL:
            reg = &device->registers.coil;
            break;
        case MODBUS_DISCRETE_INPUT:
            reg = &device->registers.discretInput;
            break;
    }

    if(reg->ptr == NULL)
    {
        result->exceptionCode = MODBUS_EXCEP_SLAVE_FAILURE;
        return MODBUS_OK;
    }

    if (args->index >= reg->len)
    {
        result->exceptionCode = MODBUS_EXCEP_ILLEGAL_ADDRESS;
        // modbus_log("%s", modbusExceptionCodeStr(MODBUS_EXCEP_ILLEGAL_ADDRESS));
        return MODBUS_OK;
    }

    uint8_t rw = 0;
    if(reg->writeMask == NULL)
        rw = 0; // no write mask provided, assume read only
    else
        rw = modbusMaskRead(reg->writeMask, args->index);
    if(args->query <= MODBUS_REGQ_W_CHECK)
        if (rw)
            modbus_log("RW");
        else
        {
            modbus_log("RO");
            if (args->query == MODBUS_REGQ_W_CHECK)
            {
                result->exceptionCode = MODBUS_EXCEP_SLAVE_FAILURE;
                // modbus_log(" -> %s", modbusExceptionCodeStr(MODBUS_EXCEP_SLAVE_FAILURE));
                return MODBUS_OK;
            }
        }
    else
    {
        if(args->query == MODBUS_REGQ_W && reg->write != NULL)
            reg->write(reg->ptr, args->index, args->value);

        if(args->query == MODBUS_REGQ_R && reg->read != NULL)
            result->value = reg->read(reg->ptr, args->index);

        if(device->hwCallback != NULL)
            device->hwCallback(device, args, result);
    }
    return MODBUS_OK;
}

ModbusError exceptionCallback(
    const ModbusSlave *slave,
    uint8_t function,
    ModbusExceptionCode code)
{
    modbusDevice_t *device = slave->context;
    modbus_log("\r\n%.2x | %-45s | %0.3d |", 
        device->address,
        modbusExceptionCodeStr(code),
        function);
    return MODBUS_OK;
}

uint8_t modbus_init(modbusController_t *controller)
{
    // Create a slave
    ModbusErrorInfo err = modbusSlaveInit(
        &controller->engine,
        registerCallback,
        exceptionCallback,
        modbusStaticAllocator,
        modbusSlaveDefaultFunctions,
        modbusSlaveDefaultFunctionCount);
    if (modbusIsOk(err))
    {
        return 0;
    }
    return -1;
}

uint8_t modbus_run(modbusController_t *controller)
{
    controller->idx += controller->read(controller,
                                        (uint8_t *)(controller->buf + controller->idx),
                                        MODBUS_RTU_ADU_MAX - controller->idx - 1);

    modbusDevice_t *device = controller->slaves;
    while (device != NULL)
    {
        controller->engine.context = device;
        ModbusErrorInfo err = modbusParseRequestRTU(&controller->engine, device->address, controller->buf, controller->idx);
        if (modbusIsOk(err))
        {
            uint16_t length = modbusSlaveGetResponseLength(&controller->engine);
            const uint8_t *const response = modbusSlaveGetResponse(&controller->engine);
            controller->write(controller, response, length);
            modbus_flush(controller);
        }
        else if (modbusGetErrorCode(err) == MODBUS_ERROR_CRC) // incomplete frame abort parsing for other addresses
        {
            break;
        }
        device = device->next;
    }
    return 0;
}

void modbus_register_platform(modbusController_t *controller, platform_modbus_read_fptr read, platform_modbus_write_fptr write)
{
    controller->read = read;
    controller->write = write;
}

void modbus_flush(modbusController_t *controller)
{
    memset(controller->buf, 0, controller->idx);
    controller->idx = 0;
    modbusSlaveFreeResponse(&controller->engine);
}

void modbus_add_device(modbusController_t *controller, modbusDevice_t *device)
{
    device->next = NULL;

    if (controller->slaves != NULL)
    {
        modbusDevice_t *last = controller->slaves;
        while (last->next != NULL)
            last = last->next;
        last->next = device;
    }
    else
    {
        controller->slaves = device;
    }
}