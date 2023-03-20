#!/usr/bin/env python

from pymodbus.client import ModbusSerialClient as ModbusClient



if __name__ == '__main__':
    import argparse
    parser = argparse.ArgumentParser(
                    prog = __file__,
                    description = 'modbus RTU client example')
    
    parser.add_argument('-p', '--port', help='serial port path', default='/dev/ttyACM0')
    parser.add_argument('-b', '--baud', help='serial port baudrate', type=int, default=115200)
    parser.add_argument('-s', '--slave', help='slave address', type=int, default=0x01)
    parser.add_argument('-d', '--debug', help='enable debug verbose', action='store_true')
    
    parser.add_argument('gpio', help="gpio", type=int)
    subparsers = parser.add_subparsers(help='action', dest='action')
    get_parser = subparsers.add_parser("get")
    set_parser = subparsers.add_parser("set")
    set_parser.add_argument('direction', help='direction', type=str, choices=['in', 'out'])
    set_parser.add_argument('value', help='value', type=int)
    args = parser.parse_args()

    import logging
    logging.basicConfig()
    log = logging.getLogger()
    log.setLevel(logging.DEBUG if args.debug else logging.INFO)
    
    with ModbusClient(method='rtu', port=args.port, baudrate=args.baud, timeout=1) as client:
        
        if args.action == "set":
            # set direction
            rr = client.write_coil(address=args.gpio, value=True if args.direction == 'out' else False, slave=args.slave)
            if rr.isError():
                log.error(f'failed to write direction')
            else:
                log.info(f'success')
            if args.direction == 'out':
                rr = client.write_coil(address=64+args.gpio, value=args.value, slave=args.slave)
                if rr.isError():
                    log.error(f'failed to write value')
                else:
                    log.info(f'success')
            else:
                rr = client.write_coil(address=32+args.gpio, value=args.value, slave=args.slave)
                if rr.isError():
                    log.error(f'failed to write pull')
                else:
                    log.info(f'success')
        else:
            rr = client.read_coils(address=64+args.gpio, slave=args.slave)
            if rr.isError():
                log.error(f'failed to write direction')
            else:
                log.info(f'{rr.bits[0]}')