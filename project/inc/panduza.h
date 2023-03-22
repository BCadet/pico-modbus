#ifndef PANDUZA_GPIO_H
#define PANDUZA_GPIO_H
#pragma once

typedef union {
    struct {
        uint32_t direction;
        uint32_t pulls;
        uint32_t values;
    } gpios;
    uint8_t reg[12];
} pza_gpio_t;

#endif