#ifndef _ELF_H_INCLUDED_
#define _ELF_H_INCLUDED_

#include "section.h"
#include "block.h"

section_list elf_read(char *buf);
block_list elf_make(section_list seclist);

#endif
