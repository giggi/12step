#include "defines.h"
#include "interrupt.h"
#include "serial.h"
#include "xmodem.h"
#include "lib.h"

//#define __ENABLE_DEBUG__
#include "debug.h"

static int
init(void)
{
    extern int erodata, data_start, edata, bss_start, ebss; /* symbols defined in ld.scr */

    memcpy(&data_start, &erodata, (long)&edata - (long)&data_start); /* setup data section on RAM   */
    memset(&bss_start, 0, (long)&ebss - (long)&bss_start);           /* setup bss section           */

    softvec_init();                                                  /* initialize soft intr vector */
    
    serial_init(SERIAL_DEFAULT_DEVICE);                              /* initialize serial device    */

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
    puts("  load: load a file by XMODEM.\n");
    puts("  dump: dump memory saved a file.\n");
    puts("   run: jamp to address saved a file.\n");
    puts(" reset: reset board.\n");
    puts("  help: this command.\n");
}

int
main(void)
{
    static char buf[16];
    static long size = -1;
    static unsigned char* loadbuf = NULL;
    extern  int buffer_start;
    char* entry_point;
    void (*f)(void);

    INTR_DISABLE;
    
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
            entry_point = elf_load(loadbuf);
            if(!entry_point){
                puts("run error!\n");
            }else{
                puts("starting from entry point: ");
                putxval((unsigned long)entry_point, 0);
                puts("\n");
                f = (void (*)(void))entry_point;
                f();
            }
        }else if(!strcmp(buf, "reset")){
            f = (void (*)(void))0x0;
            f();
        }else if(!strcmp(buf, "help")){
            print_usage();
        }else{
            puts("unknown.\n");
        }
    }
    
    return 0;
}
