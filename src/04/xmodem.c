#include "defines.h"
#include "serial.h"
#include "lib.h"
#include "xmodem.h"

//#define __ENABLE_DEBUG__
#include "debug.h"

#define XMODEM_SOH 0x01
#define XMODEM_STX 0x02
#define XMODEM_EOT 0x04
#define XMODEM_ACK 0x06
#define XMODEM_NAK 0x15
#define XMODEM_CAN 0x18
#define XMODEM_EOF 0x1a /* Ctrl-Z */

#define XMODEM_BLOCK_SIZE (128)

static int
xmodem_wait(void)
{
    volatile long cnt = 0;

    DBG(xmodem_wait0);
    while(!serial_is_recv_enable(SERIAL_DEFAULT_DEVICE)){
        if(++cnt >= 200000){
            cnt = 0;
            DBG(xmodem_wait1);
            serial_send_byte(SERIAL_DEFAULT_DEVICE, XMODEM_NAK);
            DBG(xmodem_wait2);
        }
    }
    return 0;
}

static int
xmodem_read_block(unsigned char block_number, char* buf)
{
    unsigned char c, block_num, check_sum;
    int i;

    DBG(xmodem_read_block0);
    block_num = serial_recv_byte(SERIAL_DEFAULT_DEVICE);
    DBG(xmodem_read_block1);
    if(block_num != block_number){
        return -1;
    }

    DBG(xmodem_read_block2);
    block_num ^= serial_recv_byte(SERIAL_DEFAULT_DEVICE);
    DBG(xmodem_read_block3);
    if(block_num == 0xff){
        return -1;
    }

    DBG(xmodem_read_block4);
    check_sum = 0;
    for(i=0; i<XMODEM_BLOCK_SIZE; ++i){
        c = serial_recv_byte(SERIAL_DEFAULT_DEVICE);
        *(buf++) = c;
        check_sum += c;
    }

    puts("checksum: ");putxval((unsigned long)check_sum, 0);puts("\n");
    check_sum ^= serial_recv_byte(SERIAL_DEFAULT_DEVICE);
    DBG(xmodem_read_block5);
    if(check_sum){
        return -1;
    }

    puts("read block finished: ");putxval((unsigned long)i, 2);puts("\n");
    return i;
}

long
xmodem_recv(char* buf)
{
    int r, receiving = 0;
    long size = 0;
    unsigned char c, block_number = 1;

    while(1){
        if(!receiving){
            DBG(xmodem_recv0);
            xmodem_wait();
            DBG(xmodem_recv1);
        }

        DBG(xmodem_recv2);
        c = serial_recv_byte(SERIAL_DEFAULT_DEVICE);
        DBG(xmodem_recv3);

        if(c == XMODEM_EOT){
            DBG(xmodem_recv4);
            serial_send_byte(SERIAL_DEFAULT_DEVICE, XMODEM_ACK);
            DBG(xmodem_recv5);
            break;
        }else if(c == XMODEM_CAN){
            DBG(xmodem_recv6);
            return -1;
        }else if(c == XMODEM_SOH){
            DBG(xmodem_recv7);
            ++receiving;
            r = xmodem_read_block(block_number, buf);
            if(r < 0){
                DBG(xmodem_recv8);
                serial_send_byte(SERIAL_DEFAULT_DEVICE, XMODEM_NAK);
                DBG(xmodem_recv8);
            }else{
                DBG(xmodem_recv9);
                ++block_number;
                size += r;
                buf  += r;
                serial_send_byte(SERIAL_DEFAULT_DEVICE, XMODEM_ACK);
                DBG(xmodem_recv10);
            }
        }else{
            if(receiving){
                DBG(xmodem_recv11);
                return -1;
            }
        }
    }

    DBG(xmodem_recv12);
    return size;
}
