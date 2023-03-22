#include "modbus/modbusDevice.h"
#include "modbus/modbus.h"
#include "lightmodbus/lightmodbus.h"
#include <stddef.h>

static inline void modbusWriteReg(uint8_t *ptr, uint8_t index, uint16_t value)
{
    modbusWLE(&ptr[index], value);
    modbus_log("%.4x", value);
}

static inline uint16_t modbusReadReg(uint8_t *ptr, uint8_t index)
{
    uint16_t val = modbusRLE(&ptr[index]);
    modbus_log("%.4x", val);
    return val;
}

static inline void modbusWriteBit(uint8_t *ptr, uint8_t index, uint16_t value)
{
    modbusMaskWrite(ptr, index, value);
    modbus_log("%d", value);
}

static inline uint16_t modbusReadBit(uint8_t *ptr, uint8_t index)
{
    uint16_t val = modbusMaskRead(ptr,index);
    modbus_log("%d", val);
    return val;
}

void modbusDevice_add_coilRegister(modbusDevice_t *device, uint8_t *reg, uint8_t length, uint8_t *writeMask)
{
    device->registers.coil.ptr = reg;
    device->registers.coil.len = length<<3;
    device->registers.coil.writeMask = writeMask;
    device->registers.coil.write = modbusWriteBit;
    device->registers.coil.read = modbusReadBit;
}

void modbusDevice_add_holdingRegister(modbusDevice_t *device, uint8_t *reg, uint8_t length, uint8_t *writeMask)
{
    device->registers.holding.ptr = reg;
    device->registers.holding.len = length;
    device->registers.holding.writeMask = writeMask;
    device->registers.holding.write = modbusWriteReg;
    device->registers.holding.read = modbusReadReg;
}

void modbusDevice_add_discretInputRegister(modbusDevice_t *device, uint8_t *reg, uint8_t length)
{
    device->registers.discretInput.ptr = reg;
    device->registers.discretInput.len = length<<3;
    device->registers.discretInput.writeMask = NULL;
    device->registers.discretInput.write = NULL;
    device->registers.discretInput.read = modbusReadBit;
}

void modbusDevice_add_inputRegister(modbusDevice_t *device, uint8_t *reg, uint8_t length)
{
    device->registers.coil.ptr = reg;
    device->registers.coil.len = length;
    device->registers.coil.writeMask = NULL;
    device->registers.coil.write = NULL;
    device->registers.coil.read = modbusReadReg;
}