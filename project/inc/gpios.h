#ifndef GPIOS_H
#define GPIOS_H
#pragma once

#include <stdint.h>

void gpios_update(uint32_t directions, uint32_t pulls, uint32_t *values);
void gpios_init(uint32_t mask);

#endif