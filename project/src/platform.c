#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "platform.h"
#include "tusb.h"
#include "modbus.h"
#include "hardware/timer.h"

// I2C reserves some addresses for special purposes. We exclude these from the scan.
// These are any addresses of the form 000 0xxx or 111 1xxx
bool reserved_addr(uint8_t addr) {
    return (addr & 0x78) == 0 || (addr & 0x78) == 0x78;
}


void platform_rv3028_init(void)
{
    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(PICO_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_I2C_SDA_PIN);
    gpio_pull_up(PICO_I2C_SCL_PIN);

    printf("\nI2C Bus Scan\n");
    printf("   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");

    for (int addr = 0; addr < (1 << 7); ++addr) {
        if (addr % 16 == 0) {
            printf("%02x ", addr);
        }

        // Perform a 1-byte dummy read from the probe address. If a slave
        // acknowledges this address, the function returns the number of bytes
        // transferred. If the address byte is ignored, the function returns
        // -1.

        // Skip over any reserved addresses.
        int ret;
        uint8_t rxdata;
        if (reserved_addr(addr))
            ret = PICO_ERROR_GENERIC;
        else
            ret = i2c_read_blocking(i2c1, addr, &rxdata, 1, false);

        printf(ret < 0 ? "." : "@");
        printf(addr % 16 == 15 ? "\n" : "  ");
    }
    printf("Done.\n");
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
    struct modbusController *controller = user_data;
    modbus_flush(controller);
}

uint32_t platform_modbus_read(struct modbusController *controller, uint8_t *buf, uint8_t len)
{
    uint32_t count=0;
    if (tud_cdc_n_connected(MODBUS_PORT))
    {
        if (tud_cdc_n_available(MODBUS_PORT))
        {
            count = tud_cdc_n_read(MODBUS_PORT, buf, len);
            if(controller->alarm_id > 0)
                cancel_alarm(controller->alarm_id);
            controller->alarm_id = add_alarm_in_ms(10, platform_modbus_alarm, controller, true);
        }
    }
    return count;
}

uint32_t platform_modbus_write(struct modbusController *controller, const uint8_t * const buf, uint8_t len)
{
    irq_set_enabled(PICO_STDIO_USB_LOW_PRIORITY_IRQ, false);
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
    tud_task();
    tud_cdc_n_write_flush(MODBUS_PORT);
    irq_set_enabled(PICO_STDIO_USB_LOW_PRIORITY_IRQ, true);
    // irq_set_pending(PICO_STDIO_USB_LOW_PRIORITY_IRQ);
    if(controller->alarm_id > 0)
        cancel_alarm(controller->alarm_id);
    return remaining;
}
