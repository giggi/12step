#include <stdio.h>

#include "lib.h"

int b16r(void *buf)
{
  return ((unsigned char *)buf)[0] << 8 | ((unsigned char *)buf)[1];
}

int l16r(void *buf)
{
  return ((unsigned char *)buf)[1] << 8 | ((unsigned char *)buf)[0];
}

long b32r(void *buf)
{
  return
    ((unsigned char *)buf)[0] << 24 | ((unsigned char *)buf)[1] << 16 | 
    ((unsigned char *)buf)[2] <<  8 | ((unsigned char *)buf)[3];
}

long l32r(void *buf)
{
  return
    ((unsigned char *)buf)[3] << 24 | ((unsigned char *)buf)[2] << 16 | 
    ((unsigned char *)buf)[1] <<  8 | ((unsigned char *)buf)[0];
}

void b16w(void *buf, int val)
{
  ((unsigned char *)buf)[0] = (val >> 8) & 0xff;
  ((unsigned char *)buf)[1] = (val     ) & 0xff;
}

void l16w(void *buf, int val)
{
  ((unsigned char *)buf)[1] = (val >> 8) & 0xff;
  ((unsigned char *)buf)[0] = (val     ) & 0xff;
}

void b32w(void *buf, long val)
{
  ((unsigned char *)buf)[0] = (val >> 24) & 0xff;
  ((unsigned char *)buf)[1] = (val >> 16) & 0xff;
  ((unsigned char *)buf)[2] = (val >>  8) & 0xff;
  ((unsigned char *)buf)[3] = (val      ) & 0xff;
}

void l32w(void *buf, long val)
{
  ((unsigned char *)buf)[3] = (val >> 24) & 0xff;
  ((unsigned char *)buf)[2] = (val >> 16) & 0xff;
  ((unsigned char *)buf)[1] = (val >>  8) & 0xff;
  ((unsigned char *)buf)[0] = (val      ) & 0xff;
}

char *v2sl(int val)
{
  static char buf[32];
  sprintf(buf, "0x%08x (%d)", val, val);
  return buf;
}

char *v2ss(int val)
{
  static char buf[32];
  sprintf(buf, "0x%04x (%d)", val, val);
  return buf;
}
