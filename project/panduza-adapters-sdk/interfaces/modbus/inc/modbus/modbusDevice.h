#ifndef MODBUSDEVICE_H
#define MODBUSDEVICE_H
#pragma once

#include <stdint.h>

typedef struct modbusDevice modbusDevice_t;

typedef void (*deviceHwCallback_fptr)(modbusDevice_t *, const void *, void *);

typedef struct modbusRegister
{
    uint8_t* ptr;
    uint8_t* writeMask;
    uint8_t len;
    void (*write)(uint8_t*, uint8_t, uint16_t);
    uint16_t (*read)(uint8_t*, uint8_t);
} modbusRegister_t;

struct modbusDevice
{
    uint8_t address; // device address on the bus
    struct 
    {
        modbusRegister_t coil;   // pointer to coil data register
        modbusRegister_t discretInput;   // pointer to discret input data register
        modbusRegister_t holding; // pointer to holding data register
        modbusRegister_t input; // pointer to input data register
    } registers;
    deviceHwCallback_fptr hwCallback;
    modbusDevice_t *next; // pointer to next device
};

void modbusDevice_add_coilRegister(modbusDevice_t *device, uint8_t *reg, uint8_t length, uint8_t *writeMask);
void modbusDevice_add_holdingRegister(modbusDevice_t *device, uint8_t *reg, uint8_t length, uint8_t *writeMask);
void modbusDevice_add_discretInputRegister(modbusDevice_t *device, const uint8_t *const reg, uint8_t length);
void modbusDevice_add_inputRegister(modbusDevice_t *device, const uint8_t * const reg, uint8_t length);

#endif