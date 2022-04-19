# Drivers for Water Supply System

## xl9555

### Brief Introduction
> &ensp;&ensp;The xl9555 is a 16-pin gpio expansion chip integrated with an I2C slave bus interface consisting of pairs of registers named INPUT, OUTPUT, INV, DIRECTION.

### Code Logic
> &ensp;&ensp;Config INV and DIRECTION registers, then read from the INPUT registers or write to the output registers

### Test Case
```
i2c=I2C(0,scl=Pin(18),sda=Pin(19),freq=400*1000)
xl9555_inst=xl9555(i2c)
flag=False
while True:
    xl9555_inst.set_dir(b'\xff\x00')
    xl9555_inst.set_inv(b'\x00\x00')
    buffer=xl9555_inst.get_output()
    if(flag):
        xl9555_inst.set_output(b'\xff\xff')
    else:
        xl9555_inst.set_output(b'\x00\x00')
    flag=not flag
    buffer=xl9555_inst.get_input()
    print("input: ",buffer)
```


## ms5175

### Brief Introduction About the Chip
> &ensp;&ensp;The ms5175, an 4-channel analog to digit converter(ADC), with an I2C slave bus interface. It's a highly configurable chip with optional convertion mode, optional convertion speed, and optional gain.

### Code Logic
> &ensp;&ensp;Do the configuration in the cached config register, then flush the cached config register to the real hardware config register. After the configuration operation is done, read data out from the data register or configure the chip again.

### Test Case
```
i2c=I2C(0,scl=Pin(18),sda=Pin(19),freq=400*1000)
ms5175_inst=ms5175(i2c)
while True:
    ms5175_inst.flush_config()
    buffer=ms5175_inst.get_data()
    (data,config)=struct.unpack(">hB",buffer)
    print("[AD] data: {0:d}, config: {1:08b}".format(data,config))
```