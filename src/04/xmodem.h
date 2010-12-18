#ifndef _XMODEM_H_INCLUDED_
#define _XMODEM_H_INCLUDED_

/**
 *  XMODEM Block Format
 *
 *  +---+---+---+-----------------------------+---+
 *  | a | b | c |               d             | e |
 *  +---+---+---+-----------------------------+---+
 *
 *  a: SOH (1B)
 *  b: Number of Block (1B)
 *  c: Reversed bit of 'b' (1B)
 *  d: Block body (128B)
 *  e: Check sum (1B)
 */

long xmodem_recv(char* buf);

#endif /* _XMODEM_H_INCLUDED_ */
