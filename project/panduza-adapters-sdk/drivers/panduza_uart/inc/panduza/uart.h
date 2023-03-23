#ifndef PANDUZA_UART_H
#define PANDUZA_UART_H
#pragma once

#include "stdint.h"

typedef struct {
    uint8_t id;
} pza_uart_regs_t;

void pza_uart_init(pza_uart_regs_t *regs);
void pza_uart_run(pza_uart_regs_t *regs);

#endif