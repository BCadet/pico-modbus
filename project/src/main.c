#include "pico/stdlib.h"
#include "platform.h"
#include "rtc.h"
#include "modbus.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include <string.h>
#include "pico/bootrom.h"

void updateRegisterMap(struct modbusDevice *device)
{
    device->registers[0] = gpio_get(GPIO_BUTTON);
    gpio_put(PICO_DEFAULT_LED_PIN, device->registers[1]);
    pwm_set_gpio_level(28, (uint16_t)(0.05f*(float)device->registers[2] + 3289.0f) );
    memcpy(&device->registers[3], rtc_get_current_time(), 4*sizeof(uint32_t));
    if(device->registers[254] == 42) reset_usb_boot((1<<PICO_DEFAULT_LED_PIN),0);
}

int main()
{
    struct modbusController controller = {0};
    struct modbusDevice device[2] = {0};
    device[0].address = 0x01;
    device[0].WP[0] = 1;
    device[1].address = 0x02;
    stdio_usb_init();

    gpio_init(GPIO_BUTTON);
    gpio_pull_up(GPIO_BUTTON);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    gpio_set_function(28, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(28);
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 38);
    pwm_init(slice_num, &config, true);

    modbus_init(&controller);
    modbus_add_device(&controller, &device[0]);
    modbus_add_device(&controller, &device[1]);
    modbus_register_platform(&controller, platform_modbus_read, platform_modbus_write);
    rtc_init();

    while(true)
    {
        modbus_run(&controller);
        rtc_update();
        updateRegisterMap(&device[0]);
    }
    return 0;
}
