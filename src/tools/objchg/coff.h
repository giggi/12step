#ifndef _COFF_H_INCLUDED_
#define _COFF_H_INCLUDED_

#include "section.h"
#include "block.h"

section_list coff_read(char *buf);
block_list coff_make(section_list seclist);

#endif
