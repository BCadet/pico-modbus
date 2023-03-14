#include "pico/stdlib.h"
#include "platform.h"
#include "rtc.h"
#include "modbus.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"
#include <stdio.h>
#include <string.h>
#include "pico/bootrom.h"

void updateRegisterMap(struct registerMap *map)
{
    map->registers[0] = gpio_get(GPIO_BUTTON);
    gpio_put(PICO_DEFAULT_LED_PIN, map->registers[1]);
    pwm_set_gpio_level(28, (uint16_t)(0.05f*(float)map->registers[2] + 3289.0f) );
    memcpy(&map->registers[3], rtc_get_current_time(), 4*sizeof(uint32_t));
    if(map->registers[254] == 42) reset_usb_boot((1<<PICO_DEFAULT_LED_PIN),0);
}

int main()
{
    struct modbus slave = {0};
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

    modbus_init(&slave);
    modbus_register_platform(&slave, platform_modbus_read, platform_modbus_write);
    rtc_init();

    while(true)
    {
        modbus_run(&slave);
        rtc_update();
        updateRegisterMap(&slave.map);
    }
    return 0;
}
