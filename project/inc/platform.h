#ifndef PLATFORM_H
#define PLATFORM_H
#pragma once

#define MODBUS_PORT 1
#define GPIO_BUTTON 12

void platform_rv3028_init(void);
uint8_t platform_rv3028_read(uint8_t Device_Addr, uint8_t Reg_Addr, uint8_t* p_Reg_Data, uint32_t Length);
uint8_t platform_rv3028_write(uint8_t Device_Addr, uint8_t Reg_Addr, const uint8_t* p_Reg_Data, uint32_t Length);

void platform_modbus_usb_cdc_xfer(void);

#endif