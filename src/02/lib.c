#include "defines.h"
#include "serial.h"
#include "lib.h"

void* memset(void* b, int c, long len)
{
    char* p;
    for(p=b; len>0; --len){
        *(p++) = c;
    }
    return b;
}

void* memcpy(void* dst, const void* src, long len)
{
    char* d = dst;
    const char* s = src;
    for(; len>0; --len){
        *(d++) = *(s++);
    }
    return src;
}

void* memcmp(const void* b1, const void* b2, long len)
{
    const char* p1 = b1, p2 = b2;
    for(; len>0; --len){
        if(*p1 != *p2){
            return (*p1 > *p2)?1:-1;
        }
        ++p1;
        ++p2;
    }
    return 0;
}

int putc(unsigned char c)
{
    if(c == '\n'){
        serial_send_byte(SERIAL_DEFAULT_DEVICE, '\r');
    }
    return serial_send_byte(SERIAL_DEFAULT_DEVICE, c);
}

int puts(unsigned char* str){
    while(*str){
        putc(*(str++));
    }
    return 0;
}

