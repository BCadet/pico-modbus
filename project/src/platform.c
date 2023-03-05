#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "platform.h"
#include "tusb.h"
#include "modbus.h"

void platform_rv3028_init(void)
{
    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(PICO_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_I2C_SDA_PIN);
    gpio_pull_up(PICO_I2C_SCL_PIN);
}

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

void platform_modbus_usb_cdc_xfer(void)
{
    uint8_t itf;
    uint32_t count;
    static char in[256];
    static int idx = 0;
    ModbusErrorInfo err;

    // connected() check for DTR bit
    // Most but not all terminal client set this when making connection
    if (tud_cdc_n_connected(MODBUS_PORT))
    {
        if (tud_cdc_n_available(MODBUS_PORT))
        {
            count = tud_cdc_n_read(MODBUS_PORT, in + idx, 256 - idx);
            idx += count;
            err = modbusParseRequestRTU(get_modbus_slave(), 0x01, in, idx);
            if (modbusIsOk(err))
            {
                tud_cdc_n_write(
                    MODBUS_PORT,
                    modbusSlaveGetResponse(get_modbus_slave()),
                    modbusSlaveGetResponseLength(get_modbus_slave()));
                tud_cdc_n_write_flush(MODBUS_PORT);
                memset(in, 0, 256);
                idx = 0;
            }
        }
    }
}
