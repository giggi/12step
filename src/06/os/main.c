#include "defines.h"
#include "serial.h"
#include "lib.h"

//#define __ENABLE_DEBUG__
#include "debug.h"

static void
print_usage(void)
{
    puts("commands: \n");
    puts("  echo: call back strings.\n");
    puts("  exit: exit process.\n");
    puts(" reset: rest board.\n");
    puts("  help: this command.\n");
}

int
main(void)
{
    static char buf[32];
    void (*f)(void);
    
    puts("Hello World!\n");

    while(1){
        puts("> ");
        gets(buf);

        if(!strncmp(buf, "echo", 4)){
            puts(buf+4);
            puts("\n");
        }else if(!strcmp(buf, "exit")){
            break;
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
