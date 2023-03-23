#ifndef PANDUZA_CONN_MODBUS_H
#define PANDUZA_CONN_MODBUS_H
#pragma once

#include "modbus/modbus.h"

uint32_t platform_modbus_write(struct modbusController *controller, const uint8_t *const buf, uint8_t len);
uint32_t platform_modbus_read(struct modbusController *controller, uint8_t *buf, uint8_t len);

#endif