#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "platform.h"
#include "tusb.h"
#include "modbus.h"
#include "hardware/timer.h"

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

int64_t platform_modbus_alarm(alarm_id_t alarm_num, void *user_data)
{
    struct modbus *slave = user_data;
    modbus_flush(slave);
}

uint32_t platform_modbus_read(struct modbus *drv, uint8_t *buf, uint8_t len)
{
    uint32_t count=0;
    if (tud_cdc_n_connected(MODBUS_PORT))
    {
        if (tud_cdc_n_available(MODBUS_PORT))
        {
            count = tud_cdc_n_read(MODBUS_PORT, buf, len);
            if(drv->alarm_id != 0)
                cancel_alarm(drv->alarm_id);
            drv->alarm_id = add_alarm_in_ms(10, platform_modbus_alarm, drv, true);
        }
    }
    return count;
}

uint32_t platform_modbus_write(struct modbus *drv, const uint8_t * const buf, uint8_t len)
{
    uint16_t remaining = len;
    do
    {
        uint32_t ret = tud_cdc_n_write(
            MODBUS_PORT,
            buf + (len-remaining),
            remaining > CFG_TUD_CDC_TX_BUFSIZE ? CFG_TUD_CDC_TX_BUFSIZE : remaining
            );
        remaining -= ret;
    } while(remaining);
    tud_cdc_n_write_flush(MODBUS_PORT);
    if(drv->alarm_id != 0)
        cancel_alarm(drv->alarm_id);
    return remaining;
}
