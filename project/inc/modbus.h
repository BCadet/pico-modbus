#ifndef MODBUS_H
#define MODBUS_H
#pragma once

#define LIGHTMODBUS_SLAVE
#include "lightmodbus/lightmodbus.h"

struct modbus;
typedef uint32_t(*platform_modbus_read_fptr)(struct modbus*, uint8_t*, uint8_t);
typedef uint32_t(*platform_modbus_write_fptr)(struct modbus*, const uint8_t* const, uint8_t);

struct registerMap
{
    uint16_t registers[255];
    uint8_t WP[255];
};

struct modbus
{
    ModbusSlave slave;
    struct registerMap map;
    uint8_t buf[255];
    uint32_t idx;
    platform_modbus_read_fptr read;
    platform_modbus_write_fptr write;
    uint8_t alarm_id;
};

uint8_t modbus_init(struct modbus *drv);
uint8_t modbus_run(struct modbus *drv);
void modbus_flush(struct modbus *drv);
void modbus_register_platform(struct modbus *drv, platform_modbus_read_fptr read, platform_modbus_write_fptr write);

#endif