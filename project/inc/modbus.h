#ifndef MODBUS_H
#define MODBUS_H
#pragma once

#define LIGHTMODBUS_SLAVE
#include "lightmodbus.h"

ModbusSlave* get_modbus_slave(void);
void modbus_init(void);

#endif