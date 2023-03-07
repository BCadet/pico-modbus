#include "pico/stdlib.h"
#include "platform.h"
#include "rtc.h"
#include "modbus.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

int main()
{
    stdio_usb_init();

    gpio_init(GPIO_BUTTON);
    gpio_pull_up(GPIO_BUTTON);

    gpio_set_function(PICO_DEFAULT_LED_PIN, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(PICO_DEFAULT_LED_PIN);
    pwm_config config = pwm_get_default_config();
    float div = ((float)clock_get_hz(clk_sys) / 50.0f);
    pwm_config_set_clkdiv(&config, div);
    pwm_init(slice_num, &config, true);

    modbus_init();
    rtc_init();

    while(true)
    {
        platform_modbus_usb_cdc_xfer();
        rtc_update();
    }
    return 0;
}
