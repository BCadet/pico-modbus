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
        # self.log.setLevel(logging.DEBUG)
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
        buff = buffer
        if len(buff)%2 == 1:
            buff += b'\x00'
        array = struct.unpack(f"<{len(buff)}B", buff)
        buff16 = [v1 | (v2 << 8) for v1, v2 in zip(array[::2], array[1::2])]
        self.client.write_registers(0, address, count=1, slave=self.slave)
        self.client.write_registers(3, buff16, count=len(buff16), slave=self.slave)
        self.client.write_registers(1, end-start, count=1, slave=self.slave)

    def readfrom_into(self, address, buffer, *, start=0, end=None, stop=True):
        """Read data from an address and into the buffer"""
        self.log.debug(f'readfrom_into  {address=} {buffer=} {start=} {end=}')
        self.log.debug(f'{len(buffer)=}')
        end = end if end else len(buffer)
        self.client.write_registers(0, address, count=1, slave=self.slave)
        self.client.write_registers(2, end-start, count=1, slave=self.slave)
        count = (end-start)//2
        result = self.client.read_input_registers(3, count=count+1, slave=self.slave)
        if result.isError():
            self.log.error(result)
            raise Exception
        else:
            buff = struct.pack(f"<{len(result.registers)}H", *result.registers)
            buffer[start:] = buff[:-start]


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
        self.writeto(address, buffer_out, start=out_start, end=out_end)
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
    logging.basicConfig(
        level=logging.DEBUG if args.debug else logging.INFO,
        format="[%(asctime)s] %(name)15s [%(levelname)8s] --- %(message)s"
    )
    log = logging.getLogger(__file__)
    # log.setLevel(logging.DEBUG if args.debug else logging.INFO)
    
    with ModbusClient(method='rtu', port=args.port, baudrate=args.baud, timeout=1) as client:
        i2c = ModbusI2CBUS(client, args.slave)
        rtc = adafruit_pcf8523.PCF8523(i2c)
        # t = time.struct_time((2023,3,30,15,6,1,1,9,-1))
        # print(t)
        # rtc.datetime = t
        # print(rtc.datetime)
        try:
            while True:
                # rtc.datetime = t
                print(rtc.datetime)
                time.sleep(1)
                # print(t.tm_hour, t.tm_min)
        except KeyboardInterrupt:
            exit()
                