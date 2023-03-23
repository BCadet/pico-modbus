#include "panduza/dio.h"
#include "connecteurs/dio.h"

#define PZA_DIO_ADDRESS 0x01

static const uint8_t pza_dio_magic[] = "PZADIO";

void pza_dio_init(pza_dio_regs_t *regs, uint32_t mask)
{
    regs->id = PZA_DIO_ADDRESS;
    memcpy(regs->identifier.content.magic, pza_dio_magic, sizeof(pza_dio_magic));
    gpios_init(mask);
}

void pza_dio_run(pza_dio_regs_t *regs)
{
    regs->inputs.content.inputs = gpios_update(
        regs->control.gpios.direction,
        regs->control.gpios.pulls,
        regs->control.gpios.values);
}