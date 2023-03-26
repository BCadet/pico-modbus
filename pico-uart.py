#!/usr/bin/env python

from pymodbus.client import ModbusSerialClient as ModbusClient
from pymodbus.exceptions import ModbusException

if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(
                    prog = __file__,
                    description = 'modbus RTU client example')
    
    parser.add_argument('-p', '--port', help='serial port path', default='/dev/ttyACM0')
    parser.add_argument('-b', '--baud', help='serial port baudrate', type=int, default=115200)
    parser.add_argument('-s', '--slave', help='slave address', type=int, default=0x02)
    parser.add_argument('-d', '--debug', help='enable debug verbose', action='store_true')
    
    subparsers = parser.add_subparsers(help='action', dest='action')
    print_parser = subparsers.add_parser("print")
    read_parser = subparsers.add_parser("read")
    print_parser.add_argument('string', help='string', type=str, nargs='+')
    read_parser.add_argument('len', help='len', type=int)
    args = parser.parse_args()

    import logging
    logging.basicConfig(format="[%(asctime)s] %(name)15s [%(levelname)8s] --- %(message)s")
    log = logging.getLogger(__file__)
    log.setLevel(logging.DEBUG if args.debug else logging.INFO)
    
    with ModbusClient(method='rtu', port=args.port, baudrate=args.baud, timeout=1) as client:
        
        if args.action == "print":
            buffer = bytes(' '.join(args.string), 'ascii')
            rr = client.write_registers(address=5, values=buffer, slave=args.slave)
            if rr.isError():
                log.error(f'{ModbusException(rr)}')
            else:
                log.info(f'success')
        else:
            rr = client.read_discrete_inputs(address=args.gpio, slave=args.slave)
            if rr.isError():
                log.error(f'failed to write direction')
            else:
                log.info(f'{rr.bits[0]}')