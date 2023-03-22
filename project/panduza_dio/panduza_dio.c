#include "panduza/dio.h"

#define PZA_DIO_ADDRESS 0x01

static const uint8_t pza_dio_magic[] = "PZADIO";

void pza_dio_init(pza_dio_regs_t *regs)
{
    regs->id = PZA_DIO_ADDRESS;
    memcpy(regs->identifier.content.magic, pza_dio_magic, sizeof(pza_dio_magic));
}

