#include <pico/stdlib.h>

void gpios_update(uint32_t directions, uint32_t pull, uint32_t *values)
{
    gpio_set_dir_all_bits(directions);
    gpio_put_all(*values);
    *values = gpio_get_all();
}

void gpios_init(void)
{
    gpio_init_mask(0x3FFFFFFF);
}