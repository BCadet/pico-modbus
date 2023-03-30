#!/usr/bin/env python

from pymodbus.client import ModbusSerialClient as ModbusClient
from pymodbus.exceptions import ModbusException
import adafruit_pcf8523
import time
import sys
import logging
import struct

class ModbusI2CBUS:
    """Custom I2C Class"""

    # pylint: disable=unused-argument
    def __init__(self, client=None, slave=0x01):
        self.client = client
        self.slave = slave
        self.log = logging.getLogger('ModbusI2CBUS')
        self.log.setLevel(logging.DEBUG)
        self.log.debug('init')
        
    def try_lock(self):
        return True
    
    def unlock(self):
        return True
        
    def scan(self):
        self.log.debug('scan')
        """Perform an I2C Device Scan"""
        return [addr for addr in range(0x79)] # HACK: all addresses are valid (la flem !)

    def writeto(self, address, buffer, *, start=0, end=None, stop=True):
        """Write data from the buffer to an address"""
        self.log.debug(f'writeto {address=} {buffer=} {start=} {end=}')
        if buffer == b'':
            raise OSError # raise OSError because pymodbus doesn't support write of enpty buffer
        end = end if end else len(buffer)
        
        #convert to 16 bits array
        buffer16 = struct.pack(f"<{end-start+1//2}H", *struct.unpack(f"<{end-start}B", buffer[start:end]))
        self.client.write_registers(0, address, count=1, slave=self.slave)
        self.client.write_registers(3, buffer16, count=len(buffer16), slave=self.slave)
        self.client.write_registers(1, end-start, count=1, slave=self.slave)

    def readfrom_into(self, address, buffer, *, start=0, end=None, stop=True):
        """Read data from an address and into the buffer"""
        self.log.debug(f'readfrom_into  {address=} {buffer=} {start=} {end=}')
        end = end if end else len(buffer)
        self.client.write_registers(0, address, count=1, slave=self.slave)
        self.client.write_registers(2, end-start, count=1, slave=self.slave)
        count = (end-start)//2 if (end-start)//2 else 1
        result = self.client.read_input_registers(3, count=count+1, slave=self.slave)
        if result.isError():
            self.log.error(result)
            raise Exception
        else:
            buffer[start:end] = struct.pack(f"<{len(result.registers)}H", *result.registers)
        self.log.debug(f'{buffer=}')

    # pylint: disable=unused-argument
    def writeto_then_readfrom(
        self,
        address,
        buffer_out,
        buffer_in,
        *,
        out_start=0,
        out_end=None,
        in_start=0,
        in_end=None,
        stop=False,
    ):
        """Write data from buffer_out to an address and then
        read data from an address and into buffer_in
        """
        self.log.debug('writeto_then_readfrom')
        out_end = out_end if out_end else len(buffer_out)
        in_end = in_end if in_end else len(buffer_in)
        result = self.writeto(address, buffer_out, start=out_start, end=out_end)
        self.readfrom_into(address, buffer_in, start=in_start, end=in_end)


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
    set_time = subparsers.add_parser("set_time")
    get_time = subparsers.add_parser("get_time")
    set_time.add_argument('time', help='time', type=str)
    args = parser.parse_args()

    import logging
    logging.basicConfig(format="[%(asctime)s] %(name)15s [%(levelname)8s] --- %(message)s")
    log = logging.getLogger(__file__)
    log.setLevel(logging.DEBUG if args.debug else logging.INFO)
    
    with ModbusClient(method='rtu', port=args.port, baudrate=args.baud, timeout=1) as client:
        i2c = ModbusI2CBUS(client, args.slave)
        rtc = adafruit_pcf8523.PCF8523(i2c)
        rtc.datetime = time.struct_time((2017,1,9,15,6,0,0,9,-1))
        try:
            while True:
                t = rtc.datetime
                print(t)
                print(t.tm_hour, t.tm_min)
        except KeyboardInterrupt:
            exit()
                