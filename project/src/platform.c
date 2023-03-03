#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

uint8_t platform_rv3028_read(uint8_t Device_Addr, uint8_t Reg_Addr, uint8_t* p_Reg_Data, uint32_t Length)
{
    uint8_t ret = i2c_write_blocking(i2c1, Device_Addr, &Reg_Addr, 1, true);
    ret |= i2c_read_blocking(i2c1, Device_Addr, p_Reg_Data, Length, false);
    return 0;
}


uint8_t platform_rv3028_write(uint8_t Device_Addr, uint8_t Reg_Addr, const uint8_t* p_Reg_Data, uint32_t Length)
{
    uint8_t ret = i2c_write_blocking(i2c1, Device_Addr, &Reg_Addr, 1, true);
    ret |= i2c_write_blocking(i2c1, Device_Addr, p_Reg_Data, Length, false);
    return 0;
}