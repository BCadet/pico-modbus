#include "pico/stdlib.h"
#include "tusb.h"
#include <stdio.h>
#include "hardware/i2c.h"

// Include implementation here
#define LIGHTMODBUS_IMPL
// Library configuration
#define LIGHTMODBUS_SLAVE_FULL
#define LIGHTMODBUS_DEBUG
#include "lightmodbus.h"

#define MODBUS_PORT 1
#define GPIO_BUTTON 12

#include "RV3028.h"
#define PASSWORD		0x20
struct tm CurrentTime = 
{
    .tm_sec = 0,
    .tm_min = 40,
    .tm_hour = 17,
    .tm_wday = 2,
    .tm_mday = 28,
    .tm_mon = 02,
    .tm_year = 23,
};

rv3028_error_t ErrorCode;
rv3028_t RTC;
rv3028_init_t RTC_Init = {
    // Use this settings to enable the battery backup
    .BatteryMode = RV3028_BAT_DSM,
    .Resistance = RV3028_TCT_3K,
    .EnableBSIE = true,
    .EnableCharge = true,

    // Use this settings to configure the time stamp function
    //.TSMode = RV3028_TS_BAT,
    .EnableTS = true,
    //.EnableTSOverwrite = true,

    // Use this settings to enable the clock output
    .Frequency = RV3028_CLKOUT_8KHZ,
    .EnableClkOut = true,

    // Use this settings for the event input configuration
    .EnableEventInt = true,
    .EventHighLevel = false,
    .Filter = RV3028_FILTER_256HZ,

    // Set the current time
    .HourMode = RV3028_HOURMODE_24,
    .p_CurrentTime = &CurrentTime,

    // Use this settings for the Power On Reset interrupt
    .EnablePOR = false,

    // Use this settings for the password function
    .Password = PASSWORD,
};

ModbusError registerCallback(
    const ModbusSlave *slave,
    const ModbusRegisterCallbackArgs *args,
    ModbusRegisterCallbackResult *result)
{
    printf("\r\nAccess query: %s at index %d", modbusRegisterQueryStr(args->query), args->index);

    switch (args->query)
    {
    // Pretend to allow all access
    case MODBUS_REGQ_R_CHECK:
    case MODBUS_REGQ_W_CHECK:
        if(args->index > 9)
            result->exceptionCode = MODBUS_EXCEP_ILLEGAL_VALUE;
        else
            result->exceptionCode = MODBUS_EXCEP_NONE;
        break;

    case MODBUS_REGQ_R:
        if(args->index > 0 && args->index < 9)
            result->value = ((int*)&CurrentTime)[args->index-1];
        else
            result->value = gpio_get(GPIO_BUTTON);
        break;

    case MODBUS_REGQ_W:
        if(args->index == 9)
            gpio_put(PICO_DEFAULT_LED_PIN, args->value);

    default:
        break;
    }

    return MODBUS_OK;
}

ModbusError exceptionCallback(const ModbusSlave *slave, uint8_t function, ModbusExceptionCode code)
{
    printf("Slave exception %s (function %d)\n", modbusExceptionCodeStr(code), function);
    return MODBUS_OK;
}

ModbusSlave slave;

//--------------------------------------------------------------------+
// USB CDC
//--------------------------------------------------------------------+
static void cdc_task(void)
{
    uint8_t itf;
    uint32_t count;
    static char in[256];
    static int idx = 0;
    ModbusErrorInfo err;

    for (itf = 0; itf < CFG_TUD_CDC; itf++)
    {
        // connected() check for DTR bit
        // Most but not all terminal client set this when making connection
        if (tud_cdc_n_connected(itf))
        {
            if (tud_cdc_n_available(itf))
            {
                switch (itf)
                {
                case MODBUS_PORT:
                    count = tud_cdc_n_read(MODBUS_PORT, in + idx, 256 - idx);
                    idx += count;
                    err = modbusParseRequestRTU(&slave, 0x01, in, idx);
                    if (modbusIsOk(err))
                    {
                        tud_cdc_n_write(
                            MODBUS_PORT,
                            modbusSlaveGetResponse(&slave),
                            modbusSlaveGetResponseLength(&slave));
                        tud_cdc_n_write_flush(MODBUS_PORT);
                        memset(in, 0, 256);
                        idx = 0;
                    }
                    break;
                default:
                break;
                }
            }
        }
    }
}

#include "platform.h"

int main()
{
    ModbusErrorInfo err;

    tud_init(BOARD_TUD_RHPORT);
    stdio_usb_init();

    gpio_init(GPIO_BUTTON);
    gpio_pull_up(GPIO_BUTTON);
    
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    i2c_init(i2c1, 400 * 1000);
    gpio_set_function(PICO_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_I2C_SDA_PIN);
    gpio_pull_up(PICO_I2C_SCL_PIN);

    RTC.p_Read = platform_rv3028_read;
    RTC.p_Write = platform_rv3028_write;
    RTC.DeviceAddr = RV3028_ADDRESS;

    // Create a slave
    err = modbusSlaveInit(
        &slave,
        registerCallback,
        exceptionCallback,
        modbusDefaultAllocator,
        modbusSlaveDefaultFunctions,
        modbusSlaveDefaultFunctionCount);

    // RV3028_DisableWP(&RTC, PASSWORD);
    printf("\r\nRTC init...");
	ErrorCode = RV3028_Init(&RTC_Init, &RTC);
    printf("\r\nErrorCode=%d", ErrorCode);
    if(ErrorCode == RV3028_NO_ERROR)
	{
	    printf("\r\nRTC initialized...");
	    printf("\r\n  HID: %x", RTC.HID);
	    printf("\r\n  VID: %x", RTC.VID);
    }

    while (true)
    {
        tud_task(); // tinyusb device task
        cdc_task();
        RV3028_GetTime(&RTC, &CurrentTime);
    }
    return 0;
}
