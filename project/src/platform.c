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

static char in[256];
static int idx = 0;
enum sm 
{
    RECEIVING,
    TIMEOUT,
} state;

void platform_modbus_alarm(uint alarm_num)
{
    if(state == RECEIVING)
    {
        add_alarm_in_us(3000, platform_modbus_alarm, NULL, true);
        state = TIMEOUT;
    }
    else
    {
        ModbusErrorInfo err = modbusParseRequestRTU(get_modbus_slave(), 0x01, in, idx);
        if (modbusIsOk(err))
        {
            uint16_t length = modbusSlaveGetResponseLength(get_modbus_slave());
            uint8_t *response = modbusSlaveGetResponse(get_modbus_slave());
            uint16_t remaining = length;
            do
            {
                uint32_t ret = tud_cdc_n_write(
                    MODBUS_PORT,
                    response + (length-remaining),
                    remaining > CFG_TUD_CDC_TX_BUFSIZE ? CFG_TUD_CDC_TX_BUFSIZE : remaining
                    );
                remaining -= ret;
            } while(remaining);
            tud_cdc_n_write_flush(MODBUS_PORT);
        }
        else
        {
            uint8_t source = modbusGetErrorSource(err);
            uint8_t err_code = modbusGetErrorCode(err);
            // printf("\r\nErrorSource=%d", source);
            // printf("\r\nErrorCode=%d", err_code);
            // printf("\r\nmodbusErrorSourceStr=%s", modbusErrorSourceStr(source));
            // printf("\r\nmodbusErrorStr=%s", modbusErrorStr(err_code));
            // printf("\r\ninvalid modbus frame: [");
            // for(int i=0; i<idx; i++) printf("%.2X", in[i]);
            // printf(" ]");
        }
            memset(in, 0, 256);
            idx = 0;
    }
    
}

void platform_modbus_usb_cdc_xfer(void)
{
    uint8_t itf;
    uint32_t count;


    // connected() check for DTR bit
    // Most but not all terminal client set this when making connection
    if (tud_cdc_n_connected(MODBUS_PORT))
    {
        if (tud_cdc_n_available(MODBUS_PORT))
        {
            state = RECEIVING;
            add_alarm_in_us(7000, platform_modbus_alarm, NULL, true);
            count = tud_cdc_n_read(MODBUS_PORT, in + idx, 256 - idx);
            idx += count;
        }
    }
}
