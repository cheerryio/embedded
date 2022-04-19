from micropython import const
from machine import I2C,Pin

MS5175_DEV_ADDR=const(0x48)

MS5175_CONFIG_STDRDY_MASK=const(1<<7)
MS5175_CONFIG_INP1_MASK=const(1<<6)
MS5175_CONFIG_INP0_MASK=const(1<<5)
MS5175_CONFIG_SC_MASK=const(1<<4)
MS5175_CONFIG_DR1_MASK=const(1<<3)
MS5175_CONFIG_DR0_MASK=const(1<<2)
MS5175_CONFIG_PGA1_MASK=const(1<<1)
MS5175_CONFIG_PGA0_MASK=const(1<<0)

class ms5175(object):
    def __init__(
        self,
        i2c_dev,
        dev_addr=MS5175_DEV_ADDR,
        input_channel=0,
        sc=0,
        convertion_speed=15,
        gain=1,
        *args,
        **kwargs
    )->None:
        self.config=0x8C
        self.i2c_dev=None
        if not isinstance(i2c_dev,I2C):
            raise ValueError("parameter is not an I2C object")
        
        self.i2c_dev=i2c_dev
        self.dev_addr=dev_addr
        self.buffer=bytearray(3)
        self.set_input_channel(input_channel)
        self.set_sc(sc)
        self.set_convertion_speed(convertion_speed)
        self.set_gain(gain)
    
    def set_config_bit(self,mask:int,value:bool)->None:
        if(value):
            self.config|= mask
        else:
            self.config&= ~mask
    
    def set_input_channel(self,channel:int)->None:
        if channel not in [0,1,2,3]:
            raise ValueError("invalid channel %d" % channel)
        if(channel==0):
            self.set_config_bit(MS5175_CONFIG_INP1_MASK,0)
            self.set_config_bit(MS5175_CONFIG_INP0_MASK,0)
        elif(channel==1):
            self.set_config_bit(MS5175_CONFIG_INP1_MASK,0)
            self.set_config_bit(MS5175_CONFIG_INP0_MASK,1)
        elif(channel==3):
            self.set_config_bit(MS5175_CONFIG_INP1_MASK,1)
            self.set_config_bit(MS5175_CONFIG_INP0_MASK,0)
        else:
            self.set_config_bit(MS5175_CONFIG_INP1_MASK,1)
            self.set_config_bit(MS5175_CONFIG_INP0_MASK,1)
    
    def set_sc(self,sc:int)->None:
        if(sc==1):
            self.set_config_bit(MS5175_CONFIG_SC_MASK,1)
        else:
            self.set_config_bit(MS5175_CONFIG_SC_MASK,0)
    
    def set_convertion_speed(self,speed:int)->None:
        if speed not in [240,60,30,15]:
            raise ValueError("invalid speed %d" % speed)
        if(speed==240):
            self.set_config_bit(MS5175_CONFIG_DR1_MASK,0)
            self.set_config_bit(MS5175_CONFIG_DR0_MASK,0)
        elif(speed==60):
            self.set_config_bit(MS5175_CONFIG_DR1_MASK,0)
            self.set_config_bit(MS5175_CONFIG_DR0_MASK,1)
        elif(speed==30):
            self.set_config_bit(MS5175_CONFIG_DR1_MASK,1)
            self.set_config_bit(MS5175_CONFIG_DR0_MASK,0)
        else:
            self.set_config_bit(MS5175_CONFIG_DR1_MASK,1)
            self.set_config_bit(MS5175_CONFIG_DR0_MASK,1)
    
    def set_gain(self,gain:int)->None:
        if gain not in [1,2,3,4]:
            raise ValueError("invalid gain %d" % gain)
        if(gain==1):
            self.set_config_bit(MS5175_CONFIG_PGA1_MASK,0)
            self.set_config_bit(MS5175_CONFIG_PGA0_MASK,0)
        elif(gain==2):
            self.set_config_bit(MS5175_CONFIG_PGA1_MASK,0)
            self.set_config_bit(MS5175_CONFIG_PGA0_MASK,1)
        elif(gain==3):
            self.set_config_bit(MS5175_CONFIG_PGA1_MASK,1)
            self.set_config_bit(MS5175_CONFIG_PGA0_MASK,0)
        else:
            self.set_config_bit(MS5175_CONFIG_PGA1_MASK,1)
            self.set_config_bit(MS5175_CONFIG_PGA0_MASK,1)
    
    def flush_config(self)->None:
        self.i2c_dev.writeto(self.dev_addr,self.config.to_bytes(1,"little"))
    
    def get_data(self)->bytearray:
        self.i2c_dev.readfrom_into(self.dev_addr,self.buffer)
        return self.buffer
    