//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
//#include <c8051f320.h>
#include <stddef.h>
#include "typedefs.h"
#include "io.h"      // To double check prototypes
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <math.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <android/log.h>
//#include "portdef.h" // Contains all port definitions

//-----------------------------------------------------------------------------
// Defines
//-----------------------------------------------------------------------------
#define READ    1
#define WRITE   0

#define IO2W_SENB0_ADDRESS 0x22
#define IO2W_SENB1_ADDRESS 0xC6

//#include "QN8035.h"

#define  LOG_TAG    "i2c"
#define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
#define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function prototypes
//-----------------------------------------------------------------------------

//void wait_us(u16 us);
void wait_ns(u16 ns);
void _nop_(void);


//-----------------------------------------------------------------------------
// Externals
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// This is used just for debugging to detect when the si47xx is not responding.
// The 47xx should always ack 2-wire transactions.  If it does not, there is a
// hardware problem that must be resolved.
//-----------------------------------------------------------------------------
static void die(void)
{
    _nop_(); // put breakpoint here during debug.
}

//-----------------------------------------------------------------------------
// Send a 2-wire start and address byte. LSB will be set to indicate a read,
// cleared to indicate a write.
//
// A START condition is defined as a high to low transition on the SDIO pin
// while SCLK is high.
//
// Inputs:
//      io2w_address:  Address of device to access.
//      operation: Set to 1 for a read, set to 0 for a write
//
// Pin drive status upon exit:
//      SDIO = high (open-drain input)
//      SCLK = high
//-----------------------------------------------------------------------------
static void io2w_start(u8 io2w_address, u8 operation)
{
#if 0
    i8 i;

	SCLK = 1;
	SDIO_OUT |= SDIO_HEX;  // Configure SDIO as push-pull

    // issue the START condition
    wait_us(1); // tSU:STA
    SDIO = 0;
    wait_us(1); // tHD:STA
    SCLK = 0;

    // Set the least significant bit to indicate read or write
    io2w_address = (io2w_address & 0xFE) | (operation & 0x01);

    // issue the control word (7 bit chip address + R/W* bit)
    // Note that tr:IN + tLOW + tf:IN + tHIGH = 2500 ns = 400 kHz
    for ( i = 7; i >= 0; i-- )
    {
        SCLK = 0;
        wait_us(1); // tf:IN
        SDIO = ((io2w_address >> i) & 0x01);
        wait_us(1); // tLOW
        SCLK = 1;
        wait_us(1); // tf:IN + tHIGH
    }

    // check the acknowledge
	SDIO_OUT &= ~(SDIO_HEX);   // Configure SDIO as open-drain

    SCLK = 0;
    SDIO = 1;
    wait_us(2); // tf:IN + tLOW
    SCLK = 1;
    wait_us(1); // tf:IN + tHIGH

    if (SDIO != 0)
        die();  // ack not received.  This should never happen. Device isn't responding.
#endif
}

//-----------------------------------------------------------------------------
// Send a 2-wire stop.  A STOP condition is defined as a low to high transition
// on the SDIO pin while SCLK is high.
//
// Pin drive status upon exit:
//      SDIO = high (open-drain input)
//      SCLK = high
//-----------------------------------------------------------------------------
static void io2w_stop(void)
{
#if 0
    SDIO_OUT |= SDIO_HEX;  // Configure SDIO as push-pull
	SCLK = 0;
    wait_us(2); // tf:IN + tLOW
    SDIO = 0;
    wait_us(1);
    SCLK = 1;
    wait_us(1); // tf:IN + tSU:STO
    SDIO = 1;
#endif
}

//-----------------------------------------------------------------------------
// Write one byte of data.
//
// Inputs:
//		wrdata:  Byte to be written
//
// Pin drive status upon exit:
//      SDIO = high (open-drain input)
//      SCLK = high
//-----------------------------------------------------------------------------
static void io2w_write_byte(u8 wrdata)
{
#if 0
    i8 i;

    SDIO_OUT |= SDIO_HEX;  // Configure SDIO as push-pull
    for ( i = 7; i >= 0; i-- )
    {
        SCLK = 0;
        wait_us(1); // tf:IN
        SDIO = ((wrdata >> i) & 0x01);
        wait_us(2); // tLOW
        SCLK = 1;
        wait_us(1); // tf:IN + tHIGH
    }
    // check the acknowledge
	SDIO_OUT &= ~(SDIO_HEX);  // Configure SDIO as open-drain
    SCLK = 0;
    SDIO = 1;   // Configure P0^7(SDIO) as a digital input
    wait_us(2); // tf:IN + tLOW
    SCLK = 1;
    wait_us(1); // tf:IN + tHIGH

    if (SDIO != 0)
        die();  // ack not received.  This should never happen. Device isn't responding.
#endif
}

//-----------------------------------------------------------------------------
// Read one byte of data.
//
// Inputs:
//		remaining bytes:  Zero value indicates an ack will not be sent.
//
// Outputs:
// 		Returns byte read
//
// Pin drive status upon exit:
//      SDIO = high (open-drain input) if no more bytes to be read
//      SDIO = low (open-drain input) if read is to continue
//      SCLK = high
//-----------------------------------------------------------------------------
static u8 io2w_read_byte(u8 remaining_bytes)
{
#if 0
    i8 i;
    u8 rddata = 0;

    SDIO_OUT &= ~(SDIO_HEX);  // Configure SDIO as open-drain
    for( i = 7; i >= 0; i-- )
    {
        SCLK = 0;
        SDIO = 1;                        // Configure P0^7(SDIO) as a digital input
        wait_us(1);                      // tf:IN
        wait_us(2);                      // tLOW
        SCLK = 1;
        wait_us(1);                      // tf:IN + tHIGH
        rddata = ((rddata << 1) | SDIO);
    }
    // set the acknowledge
    SCLK = 0;

    SDIO_OUT |= SDIO_HEX;   // Configure SDIO as push-pull

    if (remaining_bytes == 0)
        SDIO = 1;
    else
        SDIO = 0;

    wait_us(2); // tf:IN + tLOW
    SCLK = 1;
    wait_us(1); // tf:IN + tHIGH

    return rddata;
#endif
}

//-----------------------------------------------------------------------------
// Returns the appropriate address of the device based on the SENB state
//
// Outputs:
//      The address of the part
//-----------------------------------------------------------------------------
u8 io2w_get2wAddress()
{
#if 0
    u8 addr;
    if(SENB) {
        addr = IO2W_SENB1_ADDRESS;
    } else {
        addr = IO2W_SENB0_ADDRESS;
    }
    return addr;
#endif
    return (IO2W_SENB1_ADDRESS >> 1);

}

//-----------------------------------------------------------------------------
// Sends 2-wire start, writes an array of data, sends 2-wire stop.
//
// Inputs:
//		number_bytes: The number of bytes to write
//		data_out:     Source array for data to be written
//
//-----------------------------------------------------------------------------
u8 io2w_write(u8 number_bytes, u8 *data_out)
{
#if 0
    u8 addr;

    // issue the START condition with address lsb cleared for writes
	addr = io2w_get2wAddress();
    io2w_start(addr, WRITE);

    // loop writing all bytes in the data_out array
    while(number_bytes--)
    {
        io2w_write_byte(*data_out++);
    }

    // issue the STOP condition
    io2w_stop();
#endif

    int i2c_fd = open("/dev/i2c-2", 0, O_RDWR);
    if (i2c_fd < 0) {
        LOGE("ERR: device open failed!");
        return -1;
    }

    int err;
    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg msg;

    data.nmsgs = 1;
    data.msgs  = &msg;
    msg.addr  = io2w_get2wAddress();
    msg.flags = !I2C_M_RD;
    msg.len   = number_bytes;
    msg.buf   = data_out;

    LOGE("Wr Address: %d\n", msg.addr);
    LOGE("Wr Data: %d\n", msg.buf);
    err = ioctl (i2c_fd, I2C_RDWR, (unsigned long)&data);
    if (err < 0) {
        LOGE("ERR: I2C_WR => %d\n", err);
    }

    if (i2c_fd > 0) {
        close(i2c_fd);
        i2c_fd = -1;
    }
    return 0;
}

//-----------------------------------------------------------------------------
// Sends 2-wire start, reads an array of data, sends 2-wire stop.
//
// Inputs:
//		number_bytes: Number of bytes to be read
//		data_in:      Destination array for data read
//
//-----------------------------------------------------------------------------
u8 io2w_read(u8 number_bytes, u8 *data_in)
{
#if 0
    u8 addr;

    // issue the START condition with address lsb set for reads
	addr = io2w_get2wAddress();
    io2w_start(addr, READ);

    // loop until the specified number of bytes have been read
    while(number_bytes--)
    {
        *data_in++ = io2w_read_byte(number_bytes);
    }

    // issue the STOP condition
    io2w_stop();
#endif
    int i=0;
    int i2c_fd = open("/dev/i2c-2", 0, O_RDWR);
    if (i2c_fd < 0) {
        LOGE("ERR: device open failed!");
        return -1;
    }

    int err;
    //int len;
    //unsigned char i2c_buf[256];
    struct i2c_rdwr_ioctl_data data;
    struct i2c_msg msg[2];
    data.nmsgs = 2;
    data.msgs  = msg;

    msg[0].flags = !I2C_M_RD;
    msg[0].addr  = io2w_get2wAddress();
    msg[0].len   = 1;      //data address
    msg[0].buf   = data_in;

    msg[1].flags = I2C_M_RD;
    msg[1].addr  = io2w_get2wAddress();
    msg[1].len   = number_bytes-1;
    msg[1].buf   = data_in+1;


    err = ioctl (i2c_fd, I2C_RDWR, (unsigned long)&data);
    if (err < 0) {
        LOGE("ERR: I2C_RDWR => %d\n", err);
    }
/*
    for(i=0; i<len; i++) {
        LOGE("%02x ", i2c_buf[i]);
    }
    LOGE("\n");
*/
    if (i2c_fd > 0) {
        close(i2c_fd);
        i2c_fd = -1;
    }
    return 0;
}
