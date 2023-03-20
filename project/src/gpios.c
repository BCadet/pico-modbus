#include <pico/stdlib.h>

void gpios_update(uint32_t directions, uint32_t pulls, uint32_t *values)
{
    gpio_set_dir_all_bits(directions);
    for(uint8_t i=0; i<30; i++)
    {
        if(directions&(1<<i)) // disable if gpio is output
            gpio_set_pulls(i, false, false);
        gpio_set_pulls(i, (pulls & (1<<i)), !(pulls & (1<<i)));
    }
    gpio_put_masked(directions, *values);
    *values = gpio_get_all();
}

void gpios_init(uint32_t mask)
{
    gpio_init_mask(mask);
}