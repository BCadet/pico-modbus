// Include implementation here
#define LIGHTMODBUS_IMPL
// Library configuration
#define LIGHTMODBUS_SLAVE_FULL
#define LIGHTMODBUS_DEBUG
#include "lightmodbus/lightmodbus.h"

#include "modbus.h"
#include "pico/stdlib.h"
#include "platform.h"
#include <stdio.h>
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
    struct modbusDevice *device = slave->context;
    modbus_log("\r\n%.2x | %-23s | %-19s | %0.3d | ", device->address, modbusDataTypeStr(args->type), modbusRegisterQueryStr(args->query), args->index);

    result->exceptionCode = MODBUS_EXCEP_NONE;
    uint8_t rw = 0;

    if (!(device->accessTypeMask & args->type))
    {
        result->exceptionCode = MODBUS_EXCEP_ILLEGAL_FUNCTION;
        printf("%s", modbusExceptionCodeStr(MODBUS_EXCEP_ILLEGAL_FUNCTION));
        return MODBUS_OK;
    }

    if(args->query <= MODBUS_REGQ_W_CHECK) // MODBUS_REGQ_R_CHECK || MODBUS_REGQ_W_CHECK
    {
        // check if index is valid and
        // get read/write authorization
        // 0: RO
        // 1: RW
        switch (args->type)
        {
        case MODBUS_HOLDING_REGISTER:
        case MODBUS_INPUT_REGISTER:
            if (args->index >= (device->dataLen >> 1))
            {
                result->exceptionCode = MODBUS_EXCEP_ILLEGAL_ADDRESS;
                modbus_log("%s", modbusExceptionCodeStr(MODBUS_EXCEP_ILLEGAL_ADDRESS));
                return MODBUS_OK;
            }
            rw = (device->writableMask[args->index] == 0xFF);
            break;

        case MODBUS_COIL:
        case MODBUS_DISCRETE_INPUT:
            if (args->index >= (device->dataLen << 3))
            {
                result->exceptionCode = MODBUS_EXCEP_ILLEGAL_ADDRESS;
                modbus_log("%s", modbusExceptionCodeStr(MODBUS_EXCEP_ILLEGAL_ADDRESS));
                return MODBUS_OK;
            }
            rw = modbusMaskRead(device->writableMask, args->index);
            break;
        }

        if (rw)
            modbus_log("RW");
        else
        {
            modbus_log("RO");
            if (args->query == MODBUS_REGQ_W_CHECK)
            {
                result->exceptionCode = MODBUS_EXCEP_SLAVE_FAILURE;
                modbus_log(" -> %s", modbusExceptionCodeStr(MODBUS_EXCEP_SLAVE_FAILURE));
            }
        }
    }
    else // MODBUS_REGQ_W || MODBUS_REGQ_R
    {
        if(args->query == MODBUS_REGQ_W)
            if (args->type >= MODBUS_COIL)
            {
                modbusMaskWrite(device->data.u8, args->index, args->value);
                modbus_log("%d", args->value);
            }
            else
            {
                device->data.u16[args->index] = args->value;
                modbus_log("%.4x", args->value);
            }

        if(args->query == MODBUS_REGQ_R)
            if (args->type >= MODBUS_COIL)
            {
                result->value = modbusMaskRead(device->data.u8, args->index);
                modbus_log("%d", result->value);
            }
            else
            {
                result->value = device->data.u16[args->index];
                modbus_log("%.4x", result->value);
            }

        if(device->hwCallback != NULL)
        {
            device->hwCallback(device, args, result);
        }
    }

    return MODBUS_OK;
}

uint8_t modbus_init(struct modbusController *controller)
{
    // Create a slave
    ModbusErrorInfo err = modbusSlaveInit(
        &controller->engine,
        registerCallback,
        NULL,
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
                                        (uint8_t *)(controller->buf + controller->idx),
                                        MODBUS_RTU_ADU_MAX - controller->idx - 1);

    struct modbusDevice *device = controller->slaves;
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

    if (controller->slaves != NULL)
    {
        struct modbusDevice *last = controller->slaves;
        while (last->next != NULL)
            last = last->next;
        last->next = device;
    }
    else
    {
        controller->slaves = device;
    }
}