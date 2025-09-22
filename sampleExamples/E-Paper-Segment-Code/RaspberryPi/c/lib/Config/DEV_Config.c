/*****************************************************************************
* | File      	:   DEV_Config.c
* | Author      :   Waveshare team
* | Function    :   Hardware underlying interface
* | Info        :
*----------------
* |	This version:   V2.0
* | Date        :   2019-07-08
* | Info        :   Basic version
*
******************************************************************************/
#include "DEV_Config.h"
#include <unistd.h>
#include <fcntl.h>


#if USE_DEV_LIB
int GPIO_Handle;
int SPI_Handle;
int fd;
#endif

/**
 * GPIO
**/
int EPD_RST_PIN;
int EPD_BUSY_PIN;
UBYTE EPD_IIC_ADD = adds_com;


/******************************************************************************
function:	GPIO Function initialization and transfer
parameter:
Info:
******************************************************************************/
void DEV_GPIO_Mode(UWORD Pin, UWORD Mode)
{
    /*
        0:  INPT   
        1:  OUTP
    */
#ifdef USE_BCM2835_LIB  
    if(Mode == 0 || Mode == BCM2835_GPIO_FSEL_INPT){
        bcm2835_gpio_fsel(Pin, BCM2835_GPIO_FSEL_INPT);
    }else {
        bcm2835_gpio_fsel(Pin, BCM2835_GPIO_FSEL_OUTP);
    }
#elif USE_DEV_LIB
    if(Mode == 0 || Mode == LG_SET_INPUT){
        lgGpioClaimInput(GPIO_Handle,LFLAGS,Pin);
        // printf("IN Pin = %d\r\n",Pin);
    }else{
        lgGpioClaimOutput(GPIO_Handle, LFLAGS, Pin, LG_LOW);
        // printf("OUT Pin = %d\r\n",Pin);
    }
#endif   
}

void DEV_Digital_Write(UWORD Pin, UBYTE Value)
{
#ifdef USE_BCM2835_LIB
    bcm2835_gpio_write(Pin, Value);
    
#elif USE_DEV_LIB
    lgGpioWrite(GPIO_Handle, Pin, Value);
    
#endif
}

UBYTE DEV_Digital_Read(UWORD Pin)
{
    UBYTE Read_value = 0;
#ifdef USE_BCM2835_LIB
    Read_value = bcm2835_gpio_lev(Pin);
    
#elif USE_DEV_LIB
    Read_value = lgGpioRead(GPIO_Handle,Pin);
#endif
    return Read_value;
}


/**
 * delay x ms
**/
void DEV_Delay_ms(UDOUBLE xms)
{
#ifdef USE_BCM2835_LIB
    bcm2835_delay(xms);

#elif USE_DEV_LIB
    lguSleep(xms/1000.0);
#endif
}
/******************************************************************************
function:	I2C Function initialization and transfer
parameter:
Info:
******************************************************************************/
void DEV_I2C_Init(uint8_t Add)
{
#if DEV_I2C
    #ifdef USE_BCM2835_LIB
        printf("BCM2835 I2C Device\r\n");  
        bcm2835_i2c_begin();
        bcm2835_i2c_setSlaveAddress(Add);

    #elif USE_DEV_LIB
        fd = lgI2cOpen(1,Add,0);
        EPD_IIC_ADD = Add;
    #endif
#endif
}

void I2C_Write_Byte(uint8_t value)
{
	int ref;
    #ifdef USE_BCM2835_LIB
        // char wbuf[2]={Cmd, value};
        bcm2835_i2c_write(&value, 1);

    #elif USE_DEV_LIB
        lgI2cWriteByte(fd, value);
    #endif
}

int I2C_Read_Byte(uint8_t Cmd)
{
	int ref;
    #ifdef USE_BCM2835_LIB
        char rbuf[2]={0};
        bcm2835_i2c_read_register_rs(&Cmd, rbuf, 1);
        ref = rbuf[0];

    #elif USE_DEV_LIB
        lgI2cReadByteData(fd, Cmd);
    #endif
    return ref;
}

void I2C_SET_Addr(uint8_t Addr)
{
    #ifdef USE_BCM2835_LIB
        bcm2835_i2c_setSlaveAddress(Addr);

    #elif USE_DEV_LIB
        fd = lgI2cOpen(1,Addr,0);
        EPD_IIC_ADD = Addr;
    #endif
}


void DEV_GPIO_Init(void)
{
	EPD_RST_PIN     = 4;
	EPD_BUSY_PIN    = 17;

	DEV_GPIO_Mode(EPD_RST_PIN, 1);
	DEV_GPIO_Mode(EPD_BUSY_PIN, 0);
}


/******************************************************************************
function:	Module Initialize, the library and initialize the pins, SPI protocol
parameter:
Info:
******************************************************************************/
UBYTE DEV_Module_Init(void)
{	
#ifdef USE_BCM2835_LIB
	if(!bcm2835_init()) {
		printf("bcm2835 init failed  !!! \r\n");
		return 1;
	} else {
		printf("bcm2835 init success !!! \r\n");
	}

#elif USE_DEV_LIB
    char buffer[NUM_MAXBUF];
    FILE *fp;
	fp = popen("cat /proc/cpuinfo | grep 'Raspberry Pi 5'", "r");
    if (fp == NULL) {
        printf("It is not possible to determine the model of the Raspberry PI\n");
        return -1;
    }

    if(fgets(buffer, sizeof(buffer), fp) != NULL)  
    {
        GPIO_Handle = lgGpiochipOpen(4);
        if (GPIO_Handle < 0)
        {
            printf( "gpiochip4 Export Failed\n");
            return -1;
        }
    }
    else
    {
        GPIO_Handle = lgGpiochipOpen(0);
        if (GPIO_Handle < 0)
        {
            printf( "gpiochip0 Export Failed\n");
            return -1;
        }
    }
#endif
    // GPIO Config
	DEV_GPIO_Init();
    DEV_I2C_Init(EPD_IIC_ADD);
    printf("/***********************************/ \r\n");
	return 0;
}

/******************************************************************************
function:	Module exits, closes SPI and BCM2835 library
parameter:
Info:
******************************************************************************/
void DEV_Module_Exit(void)
{
#ifdef USE_BCM2835_LIB
    DEV_Digital_Write(EPD_RST_PIN, 0);
    bcm2835_i2c_end();
    bcm2835_close();
#elif USE_DEV_LIB
    DEV_Digital_Write(EPD_RST_PIN, 0);
    lgI2cClose(fd);
    lgGpiochipClose(GPIO_Handle);
#endif
}
