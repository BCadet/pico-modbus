#!/usr/bin/env python

from pymodbus.client import ModbusSerialClient as ModbusClient
from pymodbus.exceptions import ModbusException

import sys

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
    print_parser.add_argument('--string', help='string', type=str)
    read_parser.add_argument('len', help='len', type=int)
    args = parser.parse_args()

    import logging
    logging.basicConfig(format="[%(asctime)s] %(name)15s [%(levelname)8s] --- %(message)s")
    log = logging.getLogger(__file__)
    log.setLevel(logging.DEBUG if args.debug else logging.INFO)
    
    with ModbusClient(method='rtu', port=args.port, baudrate=args.baud, timeout=1) as client:
        
        if args.action == "print":
            if args.string is not None:
                string = ' '.join(args.string)
            else:
                string = sys.stdin.read()
            uint16_list = []
            # Loop through the ASCII string in steps of 2
            for i in range(0, len(string), 2):
                # Get the ASCII codes for the two characters
                code1 = ord(string[i])
                code2 = ord(string[i+1]) if i+1 < len(string) else 0

                # Concatenate the two uint8 values into a uint16 value
                uint16_val = (code2 << 8) | code1

                # Add the uint16 value to the list
                uint16_list.append(uint16_val)
            rr = client.write_registers(address=3, values=uint16_list, slave=args.slave)
            rr = client.write_registers(address=2, values=len(uint16_list)<<9, slave=args.slave)
            if rr.isError():
                log.error(f'{ModbusException(rr)}')
            else:
                log.info(f'success')
        else:
            rr = client.write_registers(address=2, values=args.len, slave=args.slave)
            ret = args.len
            while ret != 0:
                rr = client.read_holding_registers(address=2, count=1, slave=args.slave)
                if rr.isError():
                    log.error(f'{ModbusException(rr)}')
                ret = rr.registers[0] & 0xFF
            rr = client.read_input_registers(address=3, count=(args.len+1)>>1, slave=args.slave)
            log.info([hex(reg) for reg in rr.registers])