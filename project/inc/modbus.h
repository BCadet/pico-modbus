#ifndef MODBUS_H
#define MODBUS_H
#pragma once

#define LIGHTMODBUS_SLAVE
#include "lightmodbus/lightmodbus.h"

struct modbusController;
typedef uint32_t (*platform_modbus_read_fptr)(struct modbusController *, uint8_t *, uint8_t);
typedef uint32_t (*platform_modbus_write_fptr)(struct modbusController *, const uint8_t *const, uint8_t);

struct modbusDevice
{
    uint8_t address; // device address on the bus
    union
    {
        uint8_t *u8;   // pointer to raw data
        uint16_t *u16; // uint16_t pointers (warning: max = dataLen/2 !)
    } data;
    uint8_t dataLen;           // length of data accessible in u16 mode !
    uint8_t *writableMask;     // mask to allow write operation
    struct modbusDevice *next; // pointer to next device
};

struct modbusController
{
    ModbusSlave engine;
    struct modbusDevice *slaves;
    uint8_t buf[MODBUS_RTU_ADU_MAX];
    uint32_t idx;
    platform_modbus_read_fptr read;
    platform_modbus_write_fptr write;
    int32_t alarm_id;
};

uint8_t modbus_init(struct modbusController *controller);
uint8_t modbus_run(struct modbusController *controller);
void modbus_flush(struct modbusController *controller);
void modbus_register_platform(struct modbusController *controller, platform_modbus_read_fptr read, platform_modbus_write_fptr write);
void modbus_add_device(struct modbusController *controller, struct modbusDevice *device);

#endif