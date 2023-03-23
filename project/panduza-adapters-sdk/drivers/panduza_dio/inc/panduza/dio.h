#ifndef PANDUZA_DIO_H
#define PANDUZA_DIO_H
#pragma once

#include <stdint.h>

typedef union {
    struct {
        uint32_t direction;
        uint32_t pulls;
        uint32_t values;
    } gpios;
    uint8_t reg[12];
} pza_dio_control_t;

typedef union {
    struct {
        uint8_t magic[6];
        uint8_t IOs;
    } content;
    uint8_t reg[7];
} pza_identifier_t;

typedef union {
    struct {
        uint32_t inputs;
    } content;
    uint8_t reg[4];
} pza_input_t;

typedef struct {
    uint8_t id;
    pza_dio_control_t control;
    pza_identifier_t identifier;
    pza_input_t inputs;
} pza_dio_regs_t;

void pza_dio_init(pza_dio_regs_t *regs, uint32_t mask);
void pza_dio_run(pza_dio_regs_t *regs);

#endif 