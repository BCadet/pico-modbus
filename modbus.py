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
    import functools
    parser = argparse.ArgumentParser(
                    prog = __file__,
                    description = 'modbus RTU client example')
    
    parser.add_argument('-p', '--port', help='serial port path', default='/dev/ttyACM0')
    parser.add_argument('-b', '--baud', help='serial port baudrate', type=int, default=115200)
    parser.add_argument('-s', '--slave', help='slave address', type=int, default=0x01)
    parser.add_argument('type', help='register access type', type=ModbusAccessTypes, choices=list(ModbusAccessTypes), default=ModbusAccessTypes.coil)
    parser.add_argument('-d', '--debug', help='enable debug verbose', action='store_true')
    
    subparsers = parser.add_subparsers(help='type of operation', dest='operation')
    read_parser = subparsers.add_parser("read")
    write_parser = subparsers.add_parser("write")
    
    read_parser.add_argument("address", help="read start address", type=functools.partial(int, base=0))
    read_parser.add_argument("count", help="read count", type=functools.partial(int, base=0))
    
    write_parser.add_argument("address", help="write start address", type=functools.partial(int, base=0))
    write_parser.add_argument("buffer", help="write buffer", nargs='+', type=functools.partial(int, base=0))
    
    args = parser.parse_args()

    import logging
    logging.basicConfig()
    log = logging.getLogger()
    log.setLevel(logging.DEBUG if args.debug else logging.INFO)

    with ModbusClient(method='rtu', port=args.port, baudrate=args.baud, timeout=1) as client:
        client.connect()

        if args.operation == 'read':
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
        else:
            if args.type is ModbusAccessTypes.coil:
                rr = client.write_coil(address=args.address, values=args.buffer, slave=args.slave)
                if rr.isError():
                    log.error(f'failed to write')
                else:
                    log.info(f'{rr.bits}')
            elif args.type is ModbusAccessTypes.holding:
                log.info(args.buffer)
                rr = client.write_registers(address=args.address, values=args.buffer, slave=args.slave)
                if rr.isError():
                    log.error(f'failed to write')
                else:
                    log.info(f'written {rr.count} bytes at address {rr.address}: {args.buffer}')
            else:
                log.error("Input registers are read only !")
