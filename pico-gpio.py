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
    subparsers = parser.add_subparsers(help='direction', dest='direction')
    in_parser = subparsers.add_parser("in")
    out_parser = subparsers.add_parser("out")
    out_parser.add_argument('value', help='value', type=int) 
    in_parser.add_argument('pull', help='pull', type=int)
    
    args = parser.parse_args()

    import logging
    logging.basicConfig()
    log = logging.getLogger()
    log.setLevel(logging.DEBUG if args.debug else logging.INFO)
    
    with ModbusClient(method='rtu', port=args.port, baudrate=args.baud, timeout=1) as client:
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
            rr = client.write_coil(address=32+args.gpio, value=args.pull, slave=args.slave)
            if rr.isError():
                log.error(f'failed to write pull')
            else:
                log.info(f'success')