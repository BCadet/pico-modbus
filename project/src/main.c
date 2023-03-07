#include "pico/stdlib.h"
#include "platform.h"
#include "rtc.h"
#include "modbus.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>

void __not_in_flash_func(some_function_name)(void) {
    sleep_ms(1000);
}


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
    // pwm_config_set_wrap(&config, 9842);
    pwm_init(slice_num, &config, true);

    modbus_init();
    rtc_init();
    printf("\r\nsome_function_name=0x%X", some_function_name);
    printf("\r\nmodbus_init=0x%X", modbus_init);


    while(true)
    {
        platform_modbus_usb_cdc_xfer();
        rtc_update();
        pwm_set_chan_level(slice_num, 0, 3276 );
        some_function_name();
        // sleep_ms(500);
        // pwm_set_chan_level(slice_num, 0, 6553 );
        // sleep_ms(500);


    }
    return 0;
}
