#include <pico/stdlib.h>

void gpios_update(uint32_t directions, uint32_t *values)
{
    static uint32_t old_dir=0,old_pull=0,old_val=0;
    gpio_set_dir_all_bits(directions);
    for(uint8_t i=0; i<30; i++)
    {
        if(directions&(1<<i)) // skip if gpio is output
            continue;
        gpio_set_pulls(i, (*values & (1<<i)), !(*values & (1<<i)));
    }
    gpio_put_masked(directions, *values);
    *values = gpio_get_all();
}

void gpios_init(uint32_t mask)
{
    gpio_init_mask(mask);
}