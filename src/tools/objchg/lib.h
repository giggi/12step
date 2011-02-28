#ifndef _LIB_H_INCLUDED_
#define _LIB_H_INCLUDED_

int b16r(void *buf);
int l16r(void *buf);
long b32r(void *buf);
long l32r(void *buf);
void b16w(void *buf, int val);
void l16w(void *buf, int val);
void b32w(void *buf, long val);
void l32w(void *buf, long val);

char *v2sl(int val);
char *v2ss(int val);

#endif
