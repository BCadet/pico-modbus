#!/usr/bin/env python

from pymodbus.client import ModbusSerialClient as ModbusClient

if __name__ == '__main__':
    from enum import Enum
    class ModbusAccessTypes(Enum):
        coil='coil'
        holding='holding'
        input='input'

        def __str__(self):
            return self.value
    import argparse
    parser = argparse.ArgumentParser(
                    prog = 'modbus RTU client example',
                    description = 'What the program does',
                    epilog = 'Text at the bottom of help')
    parser.add_argument('-p', '--port', help='serial port path', default='/dev/ttyACM0')
    parser.add_argument('-b', '--baud', help='serial port baudrate', type=int, default=115200)
    parser.add_argument('-c', '--count', help='exchange length', type=int, default=0x01)
    parser.add_argument('-s', '--slave', help='slave address', type=int, default=0x01)
    parser.add_argument('-a', '--address', help='register address', type=int, default=0x00)
    parser.add_argument('-t', '--type', help='register access type', type=ModbusAccessTypes, choices=list(ModbusAccessTypes), default=ModbusAccessTypes.coil)
    parser.add_argument('-d', '--debug', help='enable debug verbose', action='store_true')

    args = parser.parse_args()

    import logging
    logging.basicConfig()
    log = logging.getLogger()
    log.setLevel(logging.DEBUG if args.debug else logging.INFO)

    with ModbusClient(method='rtu', port=args.port, baudrate=args.baud, timeout=1) as client:
        client.connect()

        if args.type is ModbusAccessTypes.coil:
            rr = client.read_coils(address=args.address, count=args.count, slave=args.slave)
            if rr.isError():
                log.error(f'failed to read')
            else:
                log.info(f'{rr.bits}')
        elif args.type is ModbusAccessTypes.holding:
            rr = client.read_holding_registers(address=args.address, count=args.count, slave=args.slave)
            if rr.isError():
                log.error(f'failed to read')
            else:
                log.info(f'{rr.registers}')
        else:
            rr = client.read_input_registers(address=args.address, count=args.count, slave=args.slave)
            if rr.isError():
                log.error(f'failed to read')
            else:
                log.info(f'{rr.registers}')