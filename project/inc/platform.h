#ifndef PLATFORM_H
#define PLATFORM_H
#pragma once

#include "modbus.h"

#define MODBUS_PORT 1
#define GPIO_BUTTON 12

void platform_rv3028_init(void);
uint8_t platform_rv3028_read(uint8_t Device_Addr, uint8_t Reg_Addr, uint8_t *p_Reg_Data, uint32_t Length);
uint8_t platform_rv3028_write(uint8_t Device_Addr, uint8_t Reg_Addr, const uint8_t *p_Reg_Data, uint32_t Length);

uint32_t platform_modbus_write(struct modbus *drv, const uint8_t *const buf, uint8_t len);
uint32_t platform_modbus_read(struct modbus *drv, uint8_t *buf, uint8_t len);

#endif