#include "pico/stdlib.h"
#include "platform.h"
#include "rtc.h"
#include "modbus.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>

int main()
{
    stdio_usb_init();

    gpio_init(GPIO_BUTTON);
    gpio_pull_up(GPIO_BUTTON);

    gpio_set_function(28, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(28);
    pwm_config config = pwm_get_default_config();
    printf("\r\nclock_get_hz(clk_sys)=%d", clock_get_hz(clk_sys));
    pwm_config_set_clkdiv(&config, 38);
    pwm_init(slice_num, &config, true);

    modbus_init();
    rtc_init();

    while(true)
    {
        platform_modbus_usb_cdc_xfer();
        rtc_update();
        pwm_set_chan_level(slice_num, 0, 3276 );
    }
    return 0;
}
