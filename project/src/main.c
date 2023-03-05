#include "pico/stdlib.h"
#include "platform.h"
#include "rtc.h"
#include "modbus.h"

int main()
{
    stdio_usb_init();

    gpio_init(GPIO_BUTTON);
    gpio_pull_up(GPIO_BUTTON);
    
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    modbus_init();
    rtc_init();

    while (true)
    {
        platform_modbus_usb_cdc_xfer();
        rtc_update();
    }
    return 0;
}
