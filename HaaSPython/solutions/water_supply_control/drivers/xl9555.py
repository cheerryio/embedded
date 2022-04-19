from micropython import const
from machine import I2C,Pin

XL9555_INPUT0_ADDR=const(0)
XL9555_INPUT1_ADDR=const(1)
XL9555_OUTPUT0_ADDR=const(2)
XL9555_OUTPUT1_ADDR=const(3)
XL9555_INV0_ADDR=const(4)
XL9555_INV1_ADDR=const(5)
XL9555_DIR0_ADDR=const(6)
XL9555_DIR1_ADDR=const(7)

XL9555_DEV_ADDR=const(0x26)

class xl9555(object):

    def __init__(
        self,
        i2c_dev,
        dev_addr=XL9555_DEV_ADDR,
        *args,
        **kwargs
    )->None:
        self.i2c_dev=None
        if not isinstance(i2c_dev,I2C):
            raise ValueError("parameter is not an I2C object")
        
        self.i2c_dev=i2c_dev
        self.dev_addr=dev_addr
        self.buffer=bytearray(2)

    def set_dir(self,command:bytearray)->None:
        self.i2c_dev.writeto_mem(self.dev_addr,XL9555_DIR0_ADDR,command,addrsize=8)

    def set_inv(self,command:bytearray)->None:
        self.i2c_dev.writeto_mem(self.dev_addr,XL9555_INV0_ADDR,command,addrsize=8)

    def get_input(self)->bytearray:
        self.i2c_dev.readfrom_mem_into(self.dev_addr,XL9555_INPUT0_ADDR,self.buffer,addrsize=8)
        return self.buffer

    def get_output(self)->bytearray:
        self.i2c_dev.readfrom_mem_into(self.dev_addr,XL9555_OUTPUT0_ADDR,self.buffer,addrsize=8)
        return self.buffer

    def set_output(self,data:bytearray)->None:
        self.i2c_dev.writeto_mem(self.dev_addr,XL9555_OUTPUT0_ADDR,data,addrsize=8)
