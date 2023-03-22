#ifndef MODBUS_H
#define MODBUS_H
#pragma once

#define LIGHTMODBUS_SLAVE
#include "lightmodbus/lightmodbus.h"

#include "modbusDevice.h"

#ifdef MODBUS_LOG
#define modbus_log(...) printf(__VA_ARGS__)
#else
#define modbus_log(...)
#endif

typedef struct modbusController modbusController_t;
typedef uint32_t (*platform_modbus_read_fptr)(modbusController_t *, uint8_t *, uint8_t);
typedef uint32_t (*platform_modbus_write_fptr)(modbusController_t *, const uint8_t *const, uint8_t);

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

uint8_t modbus_init(modbusController_t *controller);
uint8_t modbus_run(modbusController_t *controller);
void modbus_flush(modbusController_t *controller);
void modbus_register_platform(modbusController_t *controller, platform_modbus_read_fptr read, platform_modbus_write_fptr write);
void modbus_add_device(modbusController_t *controller, struct modbusDevice *device);

#endif