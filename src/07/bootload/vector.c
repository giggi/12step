#include "defines.h"

/**
 * +-----------------+---------+---------------------+
 * |     Intrrupt    |  No.    |      Address        |
 * +=================================================+
 * | Reset           |  0      | 0x000000            |
 * +-----------------+---------+---------------------+
 * | NMI             |  7      | 0x00001c            |
 * +-----------------+---------+---------------------+
 * | Trap            |  8 - 11 | 0x000020 - 0x00002c |
 * +-----------------+---------+---------------------+
 * | External        | 12 - 17 | 0x000030 - 0x000044 |
 * +----------+------+---------+---------------------+
 * | SCI0     | ERI0 | 52      | 0x0000d0            |
 * |          | RXI0 | 53      | 0x0000d4            |
 * |          | TXI0 | 54      | 0x0000d8            |
 * |          | TEI0 | 55      | 0x0000dc            |
 * +----------+------+---------+---------------------+
 * | SCI1     | ERI0 | 56      | 0x0000e0            |
 * |          | RXI0 | 57      | 0x0000e4            |
 * |          | TXI0 | 58      | 0x0000e8            |
 * |          | TEI0 | 59      | 0x0000ec            |
 * +----------+------+---------+---------------------+
 * | SCI2     | ERI0 | 60      | 0x0000f0            |
 * |          | RXI0 | 61      | 0x0000f4            |
 * |          | TXI0 | 62      | 0x0000f8            |
 * |          | TEI0 | 63      | 0x0000fc            |
 * +----------+------+---------+---------------------+
 */

extern void start(void);
extern void intr_syscall(void);
extern void intr_softerr(void);
extern void intr_serintr(void);

void (*vector[])(void) = {
    start, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    intr_syscall, intr_softerr, intr_softerr, intr_softerr,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
    intr_serintr, intr_serintr, intr_serintr, intr_serintr,
    intr_serintr, intr_serintr, intr_serintr, intr_serintr,
    intr_serintr, intr_serintr, intr_serintr, intr_serintr,
};

    
