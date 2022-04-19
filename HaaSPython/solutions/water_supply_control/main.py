#!/usr/bin/env python
# -*- encoding: utf-8 -*-
'''
@File       :    main.py
@Author     :    ethan.lcz
@version    :    1.0
@Description:    helloworld案例 - 周期性打印"helloworld"到console中
                 board.json - 硬件资源配置文件，详情请参考：https://haas.iot.aliyun.com/haasapi/index.html#/Python/docs/zh-CN/haas_extended_api/driver/driver
'''

from machine import I2C,Pin
import struct
import utime   # 延时函数在utime库中

from drivers.xl9555 import xl9555
from drivers.ms5175 import ms5175

if __name__ == '__main__':
    i2c=I2C(0,scl=Pin(18),sda=Pin(19),freq=400*1000)
    xl9555_inst=xl9555(i2c)
    ms5175_inst=ms5175(i2c)
    flag=False
    while True:
        dev_list=i2c.scan()
        xl9555_inst.set_dir(b'\xff\x00')
        xl9555_inst.set_inv(b'\x00\x00')
        buffer=xl9555_inst.get_output()
        if(flag):
            xl9555_inst.set_output(b'\xff\xff')
        else:
            xl9555_inst.set_output(b'\x00\x00')
        flag=not flag
        buffer=xl9555_inst.get_input()
        #print("input: ",buffer)

        ms5175_inst.flush_config()
        buffer=ms5175_inst.get_data()
        (data,config)=struct.unpack(">hB",buffer)
        print("[AD] data: {0:d}, config: {1:08b}".format(data,config))

        utime.sleep(1)
