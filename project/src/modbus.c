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

#define MAX_RESPONSE 255

ModbusError modbusStaticAllocator(ModbusBuffer *buffer, uint16_t size, void *context)
{
    static uint8_t static_buf[MAX_RESPONSE];
    static uint8_t allocated = 0;

    ModbusError retCode = MODBUS_OK;

    if(size != 0) // allocation request
    {
        if (size <= MAX_RESPONSE && allocated == 0) // Allocation request is within bounds
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
    struct modbus *this = slave->context;
    printf("\r\nAccess query: %s at index %d", modbusRegisterQueryStr(args->query), args->index);

    switch (args->query)
    {
    // Pretend to allow all access
    case MODBUS_REGQ_R_CHECK:
    case MODBUS_REGQ_W_CHECK:
        if(this->map.WP[args->index])
            result->exceptionCode = MODBUS_EXCEP_ILLEGAL_VALUE;
        else
            result->exceptionCode = MODBUS_EXCEP_NONE;
        break;

    case MODBUS_REGQ_R:
        result->value = this->map.registers[args->index];
        break;

    case MODBUS_REGQ_W:
        this->map.registers[args->index] = args->value;
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

uint8_t modbus_init(struct modbus *this)
{
    // Create a slave
    ModbusErrorInfo err = modbusSlaveInit(
                                &this->slave,
                                registerCallback,
                                exceptionCallback,
                                modbusStaticAllocator,
                                modbusSlaveDefaultFunctions,
                                modbusSlaveDefaultFunctionCount);
    if (modbusIsOk(err))
    {
        this->slave.context = this;
        return 0;
    }
}

uint8_t modbus_run(struct modbus *this)
{
    this->idx += this->read(this, this->buf + this->idx, 255 - this->idx);
    ModbusErrorInfo err = modbusParseRequestRTU(&this->slave, 0x01, this->buf, this->idx);
    if (modbusIsOk(err))
    {
        uint16_t length = modbusSlaveGetResponseLength(&this->slave);
        const uint8_t * const response = modbusSlaveGetResponse(&this->slave);
        this->write(this, response, length);
        modbus_flush(this);
    }
    return 0;
}

void modbus_register_platform(struct modbus *this, platform_modbus_read_fptr read, platform_modbus_write_fptr write)
{
    this->read = read;
    this->write = write;
}

void modbus_flush(struct modbus *this)
{
    memset(this->buf, 0, this->idx);
    this->idx = 0;
    modbusSlaveFreeResponse(&this->slave);
}
