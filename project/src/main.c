#include "gpios.h"
#include "hardware/clocks.h"
#include "hardware/pwm.h"
#include "modbus.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "platform.h"
#include <stdio.h>
#include <string.h>

void updateRegisterMap(uint8_t *data)
{
    if (gpio_get(GPIO_BUTTON))
        data[0] |= 0x01;
    else
        data[0] &= ~0x01;
    gpio_put(PICO_DEFAULT_LED_PIN, data[0] & (1 << 1));
    pwm_set_gpio_level(28, (uint16_t)(0.05f * (float)(*(uint16_t *)&data[2]) + 3289.0f));
    // if(device->data[254] == 42) reset_usb_boot((1<<PICO_DEFAULT_LED_PIN),0);
}

int main()
{
    struct modbusController controller = {0};
    struct modbusDevice device[2] = {0};
    uint8_t data[12] = {0};
    uint8_t writeMask[2][12] = {{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}, {0}};
    device[0].accessTypeMask = MODBUS_COIL | MODBUS_DISCRETE_INPUT;
    device[0].address = 0x01;
    device[0].data.u8 = &data;
    device[0].dataLen = sizeof(data);
    device[0].writableMask = &writeMask[0];
    device[1].address = 0x02;
    device[1].data.u8 = &data;
    device[1].dataLen = sizeof(data);
    device[1].writableMask = &writeMask[1];

    stdio_usb_init();

    gpios_init();

    // gpio_init(GPIO_BUTTON);
    gpio_pull_up(GPIO_BUTTON);

    gpio_init(PICO_DEFAULT_LED_PIN);
    // gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    // gpio_set_function(28, GPIO_FUNC_PWM);
    // uint slice_num = pwm_gpio_to_slice_num(28);
    // pwm_config config = pwm_get_default_config();
    // pwm_config_set_clkdiv(&config, 38);
    // pwm_init(slice_num, &config, true);

    modbus_init(&controller);
    modbus_add_device(&controller, &device[0]);
    modbus_add_device(&controller, &device[1]);
    modbus_register_platform(&controller, platform_modbus_read, platform_modbus_write);

    // printf("\r\n%d",gpio_get_all()&(1<<12)>>12);

    while (true)
    {
        modbus_run(&controller);
        // updateRegisterMap(&data);
        gpios_update(
            (uint32_t)(*(uint32_t*)data),
            (uint32_t)(*(uint32_t*)(data+4)),
            (uint32_t*)(data+8));
    }
    return 0;
}
