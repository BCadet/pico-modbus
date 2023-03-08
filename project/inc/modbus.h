#ifndef MODBUS_H
#define MODBUS_H
#pragma once

#define LIGHTMODBUS_SLAVE
#include "lightmodbus.h"

ModbusSlave* get_modbus_slave(void);
void modbus_init(void);

struct registerMap
{
    uint16_t registers[255];
    uint8_t WP[255];
};

#endif