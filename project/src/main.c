#include "panduza/dio.h"
#include "panduza/interface.h"
#include "panduza/platform.h"
#include "panduza/uart.h"
#include <stdio.h>

int main()
{
    pza_platform_init();
    pza_interface_init();

    // init the panduza dio modbus device and regs + bind
    pza_dio_t dio = {0};
    const uint32_t gpioMask = 
        ~((1 << 31) 
        | (1 << 30) 
        | (1 << 29) 
        | (1 << 24) 
        | (1 << 23) 
        | (1 << 15) 
        | (1 << 14) 
        | (1 << 01) 
        | (1 << 00)); // disable io 31/30/29/24/23/1/0
    pza_dio_init(&dio, gpioMask);

    // init the panduza uart modbus device and regs + bind
    pza_uart_t uart = {.control.content.baudrate = 115200};
    pza_uart_init(&uart);

    while (1)
    {
        pza_interface_run();
        pza_dio_run(&dio);
        pza_uart_run(&uart);
    }
    return 0;
}
