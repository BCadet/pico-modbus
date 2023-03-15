// Include implementation here
#define LIGHTMODBUS_IMPL
// Library configuration
#define LIGHTMODBUS_SLAVE_FULL
#define LIGHTMODBUS_DEBUG
#include "lightmodbus/lightmodbus.h"

#include "platform.h"
#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "modbus.h"

ModbusError modbusStaticAllocator(ModbusBuffer *buffer, uint16_t size, void *context)
{
    static uint8_t static_buf[MODBUS_RTU_ADU_MAX];
    static uint8_t allocated = 0;

    ModbusError retCode = MODBUS_OK;

    if(size != 0) // allocation request
    {
        if (size <= MODBUS_RTU_ADU_MAX && allocated == 0) // Allocation request is within bounds
        {
            buffer->data = (uint8_t*)&static_buf;
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
    struct modbusDevice *device = slave->context;
    printf("\r\n%#.2x | %-20s | %0.3d | ", device->address, modbusRegisterQueryStr(args->query), args->index);


    switch (args->query)
    {
    // always allow read
    case MODBUS_REGQ_R_CHECK:
        result->exceptionCode = MODBUS_EXCEP_NONE;
        printf("OK");
        break;
    // allow write if the corresponding WP is 0
    case MODBUS_REGQ_W_CHECK:
        if(device->WP[args->index])
        {
            result->exceptionCode = MODBUS_EXCEP_ILLEGAL_ADDRESS;
            printf("KO");
        }
        else
        {
            result->exceptionCode = MODBUS_EXCEP_NONE;
            printf("OK");
        }
        break;

    case MODBUS_REGQ_R:
        result->value = device->registers[args->index];
        printf("%#.4x", result->value);
        break;

    case MODBUS_REGQ_W:
        device->registers[args->index] = args->value;
        printf("%#.4x", device->registers[args->index]);
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

uint8_t modbus_init(struct modbusController *controller)
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

uint8_t modbus_run(struct modbusController *controller)
{
    controller->idx += controller->read(controller,
                                (uint8_t*)(controller->buf + controller->idx),
                                MODBUS_RTU_ADU_MAX - controller->idx - 1);

    struct modbusDevice *device = controller->slaves;
    while(device != NULL)
    {
        controller->engine.context = device;
        ModbusErrorInfo err = modbusParseRequestRTU(&controller->engine, device->address, controller->buf, controller->idx);
        if (modbusIsOk(err))
        {
            uint16_t length = modbusSlaveGetResponseLength(&controller->engine);
            const uint8_t * const response = modbusSlaveGetResponse(&controller->engine);
            controller->write(controller, response, length);
            modbus_flush(controller);
        }
        else if(modbusGetErrorCode(err) == MODBUS_ERROR_CRC) // incomplete frame abort parsing for other addresses
        {
            break;
        }
        device = device->next;
    }
    return 0;
}

void modbus_register_platform(struct modbusController *controller, platform_modbus_read_fptr read, platform_modbus_write_fptr write)
{
    controller->read = read;
    controller->write = write;
}

void modbus_flush(struct modbusController *controller)
{
    memset(controller->buf, 0, controller->idx);
    controller->idx = 0;
    modbusSlaveFreeResponse(&controller->engine);
}

void modbus_add_device(struct modbusController *controller, struct modbusDevice *device)
{
    device->next = NULL;

    if(controller->slaves != NULL)
    {
        struct modbusDevice *last = controller->slaves;
        while(last->next != NULL)
            last = last->next;
        last->next = device;
    }
    else
    {
        controller->slaves = device;
    }
}