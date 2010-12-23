#include "defines.h"
#include "serial.h"
#include "xmodem.h"
#include "lib.h"

//#define __ENABLE_DEBUG__
#include "debug.h"

static int
init(void)
{
    extern int erodata, data_start, edata, bss_start, ebss; /* symbols defined in ld.scr */

    memcpy(&data_start, &erodata, (long)&edata - (long)&data_start); /* setup data section on RAM */
    memset(&bss_start, 0, (long)&ebss - (long)&bss_start);           /* setup bss section         */

    serial_init(SERIAL_DEFAULT_DEVICE);                              /* initialize serial device */

    return 0;
}

static int
dump(char* buf, long size)
{
    long i;

    if(size < 0){
        puts("no data.\n");
        return -1;
    }
    for(i=0; i<size; ++i){
        putxval(buf[i], 2);
        if((i & 0xf) == 15){
            puts("\n");
        }else{
            if((i & 0xf) == 7){
                puts(" ");
            }
            puts(" ");
        }
    }
    puts("\n");

    return 0;
}

static void
wait(void)
{
    volatile long i;
    for(i=0; i<300000; ++i);
}

static void
print_usage(void)
{
    puts("commands: \n");
    puts("  help: this command.\n");
    puts("  load: load a file by XMODEM.\n");
    puts("  dump: dump memory.\n");
}

int
main(void)
{
    static char buf[16];
    static long size = -1;
    static unsigned char* loadbuf = NULL;
    extern  int buffer_start;

    init();

    puts("kzload (kozos boot loader) started.\n");

    while(1){
        puts("kzload> ");
        gets(buf);

        if(!strcmp(buf, "load")){
            loadbuf = (char*)&buffer_start;
            size = xmodem_recv(loadbuf);
            wait();
            if(size < 0){
                puts("\nXMODEM receive error!\n");
            }else{
                puts("\nXMODEM receive success.\n");
            }
        }else if(!strcmp(buf, "dump")){
            puts("size: ");
            putxval(size, 0);
            puts("\n");
            dump(loadbuf, size);
        }else if(!strcmp(buf, "run")){
            elf_load(loadbuf);
        }else if(!strcmp(buf, "help")){
            print_usage();
        }else{
            puts("unknown.\n");
        }
    }
    
    return 0;
}
