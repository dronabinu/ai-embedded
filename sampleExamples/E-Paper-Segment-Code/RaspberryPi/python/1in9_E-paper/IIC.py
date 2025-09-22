import time
import smbus
import gpiozero
import os


# Pin definition
RST_PIN         = 4
BUSY_PIN        = 17

GPIO_RST_PIN    = gpiozero.LED(RST_PIN)
GPIO_BUSY_PIN   = gpiozero.Button(BUSY_PIN, pull_up = False)

# address
adds_com        = 0x3C
adds_data       = 0x3D

# adds_com        = 0x3E
# adds_data       = 0x3F

iic = smbus.SMBus(1)

def digital_write(pin, value):
    if pin == RST_PIN:
        if value:
            GPIO_RST_PIN.on()
        else:
            GPIO_RST_PIN.off()

def digital_read(pin):
    if pin == BUSY_PIN:
        return GPIO_BUSY_PIN.value

def delay_ms(delaytime):
    time.sleep(delaytime / 1000.0)

def IIC_writebyte_com(value):
    iic.write_byte(adds_com, value)

def IIC_writeblock_com(register, values):
    iic.write_block_data(adds_com, register, values)

def IIC_writebyte_data(value):
    iic.write_byte(adds_data, value)

def IIC_writeblock_data(register, values):
    iic.write_block_data(adds_data, register, values)
    
def IIC_Readbyte_com(register):
    while(1):
        try:
            iic.write_byte(adds_com, register)
            x = iic.read_byte(adds_com)
            if(x != None):
                return x
        except :
            pass
    
def IIC_Readbyte_data(register):
    while(1):
        try:
            iic.write_byte(adds_data, register)
            x = iic.read_byte(adds_data)
            if(x != None):
                return x
        except :
            pass

def module_Init():
    GPIO_RST_PIN.off()
    return 0
    
def module_exit(cleanup=False):
    iic.close()
    GPIO_RST_PIN.off()
    print("close 5V, Module enters 0 power consumption ...")
    if cleanup:
        GPIO_RST_PIN.close()
        GPIO_BUSY_PIN.close()




