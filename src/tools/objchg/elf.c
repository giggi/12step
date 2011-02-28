#include <string.h>

#include "elf.h"
#include "lib.h"

struct elf_header {
  struct {
    unsigned char magic[4];
    unsigned char class;
#define ELF_CLASS_UNKNOWN 0
#define ELF_CLASS_32BIT   1
#define ELF_CLASS_64BIT   2
    unsigned char format;
#define ELF_FORMAT_UNKNOWN       0
#define ELF_FORMAT_LITTLE_ENDIAN 1
#define ELF_FORMAT_BIG_ENDIAN    2
    unsigned char version;
#define ELF_VERSION_NONE    0
#define ELF_VERSION_CURRENT 1
    unsigned char abi;
#define ELF_ABI_NONE         0
#define ELF_ABI_SYSV         0
#define ELF_ABI_NETBSD       2
#define ELF_ABI_LINUX        3
#define ELF_ABI_FREEBSD      9
#define ELF_ABI_OPENBSD     12
#define ELF_ABI_STANDALONE 255
    unsigned char abi_version;
    unsigned char reserve[7];
  } id;
  short type;
#define ELF_TYPE_UNKNOWN 0
#define ELF_TYPE_REL     1
#define ELF_TYPE_EXEC    2
#define ELF_TYPE_DYN     3
#define ELF_TYPE_CORE    4
  short arch;
#define ELF_ARCH_UNKNOWN  0
#define ELF_ARCH_I386     3
#define ELF_ARCH_MIPS     8
#define ELF_ARCH_PPC     20
#define ELF_ARCH_ARM     40
#define ELF_ARCH_SH      42
#define ELF_ARCH_H8_300  46
#define ELF_ARCH_H8_300H 47
#define ELF_ARCH_H8S     48
#define ELF_ARCH_H8_500  49
  long version;
  long entry_point;
  long program_header_offset;
  long section_header_offset;
  long flags;
  short header_size;
  short program_header_size;
  short program_header_num;
  short section_header_size;
  short section_header_num;
  short section_name_index;
};

struct elf_section_header {
  long name;
  long type;
#define ELF_SECTION_TYPE_NULL     0
#define ELF_SECTION_TYPE_PROGBITS 1
#define ELF_SECTION_TYPE_SYMTAB   2
#define ELF_SECTION_TYPE_STRTAB   3
#define ELF_SECTION_TYPE_RELA     4
#define ELF_SECTION_TYPE_HASH     5
#define ELF_SECTION_TYPE_DYNAMIC  6
#define ELF_SECTION_TYPE_NOTE     7
#define ELF_SECTION_TYPE_NOBITS   8
#define ELF_SECTION_TYPE_REL      9
  long flags;
#define ELF_SECTION_FLAG_WRITE (1 << 0)
#define ELF_SECTION_FLAG_ALLOC (1 << 1)
#define ELF_SECTION_FLAG_EXEC  (1 << 2)
#define ELF_SECTION_FLAG_MERGE (1 << 4)
#define ELF_SECTION_FLAG_STR   (1 << 5)
  long address;
  long offset;
  long size;
  long link;
  long info;
  long align;
  long entsize;
};

struct elf_program_header {
  long type;
#define ELF_SEGMENT_TYPE_NULL    0
#define ELF_SEGMENT_TYPE_LOAD    1
#define ELF_SEGMENT_TYPE_DYNAMIC 2
#define ELF_SEGMENT_TYPE_INTERP  3
#define ELF_SEGMENT_TYPE_NOTE    4
#define ELF_SEGMENT_TYPE_PHDR    6
  long offset;
  long virtual_addr;
  long physical_addr;
  long file_size;
  long memory_size;
  long flags;
#define ELF_SEGMENT_FLAG_EXEC  (1 << 0)
#define ELF_SEGMENT_FLAG_WRITE (1 << 1)
#define ELF_SEGMENT_FLAG_READ  (1 << 2)
  long align;
};

/* ELF形式の読み込み用関数群 */

static int elf_check(struct elf_header *header)
{
  if (memcmp(header->id.magic, "\x7f" "ELF", 4))
    return -1;

  return 0;
}

static long elf_get_paddr(struct elf_header *header, long virtual_addr, long size)
{
  struct elf_program_header *phdr;
  long physical_addr;
  int i;

  physical_addr = virtual_addr;

  for (i = 0; i < b16r(&header->program_header_num); i++) {
    phdr = (struct elf_program_header *)
      ((char *)header + b32r(&header->program_header_offset) +
       b16r(&header->program_header_size) * i);

    if ((virtual_addr >= b32r(&phdr->virtual_addr)) &&
	(virtual_addr + size <= b32r(&phdr->virtual_addr) + b32r(&phdr->memory_size))) {

      physical_addr = b32r(&phdr->physical_addr) + (virtual_addr - b32r(&phdr->virtual_addr));
      break;
    }
  }

  return physical_addr;
}

section_list elf_read(char *buf)
{
  section_list seclist;
  section sec;
  struct elf_header *header = (struct elf_header *)buf;
  struct elf_section_header *shdr, *shshdr;
  char *name, *s;
  int i;
  long flags;
  long paddr;

  if (elf_check(header) < 0) {
    printf("This is not ELF file.\n");
    return NULL;
  } else {
    printf("This is ELF file.\n");
  }

  seclist = section_list_create();

  printf("\nELF header information.\n");

  s = "(NULL)";
  switch (header->id.class) {
  case ELF_CLASS_32BIT:   s = "32bit";   break;
  case ELF_CLASS_64BIT:   s = "64bit";   break;
  case ELF_CLASS_UNKNOWN: s = "unknown"; break;
  default:                s = "error";   break;
  }
  printf("class       : %s\n", s);

  s = "(NULL)";
  switch (header->id.format) {
  case ELF_FORMAT_LITTLE_ENDIAN: s = "little endian"; break;
  case ELF_FORMAT_BIG_ENDIAN:    s = "big endian";    break;
  case ELF_FORMAT_UNKNOWN:       s = "unknown";       break;
  default:                       s = "error";         break;
  }
  printf("format      : %s\n", s);

  s = "(NULL)";
  switch (header->id.version) {
  case ELF_VERSION_CURRENT: s = "current"; break;
  case ELF_VERSION_NONE:    s = "none";    break;
  default:                  s = "error";   break;
  }
  printf("version     : %s (%d)\n", s, (int)header->id.version);

  s = "(NULL)";
  switch (header->id.abi) {
  case ELF_ABI_NETBSD:     s = "NetBSD";         break;
  case ELF_ABI_LINUX:      s = "Linux";          break;
  case ELF_ABI_FREEBSD:    s = "FreeBSD";        break;
  case ELF_ABI_OPENBSD:    s = "OpenBSD";        break;
  case ELF_ABI_STANDALONE: s = "StandAlone";     break;
  case ELF_ABI_NONE:       s = "none (SystemV)"; break;
  default:                 s = "error";          break;
  }
  printf("ABI         : %s\n", s);

  printf("ABI version : %d\n", header->id.abi_version);

  s = "(NULL)";
  switch (b16r(&header->type)) {
  case ELF_TYPE_REL:     s = "Relocate"; break;
  case ELF_TYPE_EXEC:    s = "Executable"; break;
  case ELF_TYPE_DYN:     s = "Dynamic Library"; break;
  case ELF_TYPE_CORE:    s = "Core file"; break;
  case ELF_TYPE_UNKNOWN: s = "unknown"; break;
  default:               s = "error"; break;
  }
  printf("type        : %s\n", s);

  s = "(NULL)";
  switch (b16r(&header->arch)) {
  case ELF_ARCH_I386:    s = "i386";    break;
  case ELF_ARCH_MIPS:    s = "MIPS";    break;
  case ELF_ARCH_PPC:     s = "PowerPC"; break;
  case ELF_ARCH_ARM:     s = "ARM";     break;
  case ELF_ARCH_SH:      s = "SH";      break;
  case ELF_ARCH_H8_300:  s = "H8/300";  break;
  case ELF_ARCH_H8_300H: s = "H8/300H"; break;
  case ELF_ARCH_H8S:     s = "H8S";     break;
  case ELF_ARCH_H8_500:  s = "H8/500";  break;
  case ELF_TYPE_UNKNOWN: s = "unknown"; break;
  default:               s = "error";   break;
  }
  printf("arch        : %s\n", s);

  printf("version               : %s\n", v2sl(b32r(&header->version)));
  printf("entry point           : %s\n", v2sl(b32r(&header->entry_point)));
  printf("program header offset : %s\n", v2sl(b32r(&header->program_header_offset)));
  printf("section header offset : %s\n", v2sl(b32r(&header->section_header_offset)));
  printf("architecture flags    : %s\n", v2sl(b32r(&header->flags)));
  printf("header size           : %s\n", v2ss(b16r(&header->header_size)));
  printf("program header size   : %s\n", v2ss(b16r(&header->program_header_size)));
  printf("program header number : %s\n", v2ss(b16r(&header->program_header_num)));
  printf("section header size   : %s\n", v2ss(b16r(&header->section_header_size)));
  printf("section header number : %s\n", v2ss(b16r(&header->section_header_num)));
  printf("section name index    : %s\n", v2ss(b16r(&header->section_name_index)));

  section_list_set_entry_point(seclist, b32r(&header->entry_point));

  printf("\nSection header information.\n");

  shshdr = (struct elf_section_header *)
    ((char *)header + b32r(&header->section_header_offset) +
     b16r(&header->section_header_size) * b16r(&header->section_name_index));

  for (i = 0; i < b16r(&header->section_header_num); i++) {
    shdr = (struct elf_section_header *)
      ((char *)header + b32r(&header->section_header_offset) +
       b16r(&header->section_header_size) * i);

    name = (char *)header + b32r(&shshdr->offset) + b32r(&shdr->name);
    flags = b32r(&shdr->flags);

    sec = section_create(name);
    section_list_insert(seclist, NULL, sec);

    printf("\n%s (", name[0] ? name : "(NULL)");

    s = "(NULL)";
    switch (b32r(&shdr->type)) {
    case ELF_SECTION_TYPE_NULL:     s = "NULL";     break;
    case ELF_SECTION_TYPE_PROGBITS: s = "PROGBITS"; break;
    case ELF_SECTION_TYPE_SYMTAB:   s = "SYMTAB";   break;
    case ELF_SECTION_TYPE_STRTAB:   s = "STRTAB";   break;
    case ELF_SECTION_TYPE_RELA:     s = "RELA";     break;
    case ELF_SECTION_TYPE_HASH:     s = "HASH";     break;
    case ELF_SECTION_TYPE_DYNAMIC:  s = "DYNAMIC";  break;
    case ELF_SECTION_TYPE_NOTE:     s = "NOTE";     break;
    case ELF_SECTION_TYPE_NOBITS:   s = "NOBITS";   break;
    case ELF_SECTION_TYPE_REL:      s = "REL";      break;
    default:                        s = "unknown";  break;
    }
    printf("%s)\n", s);

    printf("addr   :0x%08lx  ", b32r(&shdr->address));
    printf("offset :0x%08lx  ", b32r(&shdr->offset));
    printf("size   :0x%08lx  ", b32r(&shdr->size));
    printf("link   :0x%08lx\n", b32r(&shdr->link));
    printf("info   :0x%08lx  ", b32r(&shdr->info));
    printf("align  :0x%08lx  ", b32r(&shdr->align));
    printf("entsize:0x%08lx\n", b32r(&shdr->entsize));

    printf("flags  :0x%08lx (", flags);
    if (flags & ELF_SECTION_FLAG_WRITE) {
      printf(" WRITE");
    }
    if (flags & ELF_SECTION_FLAG_ALLOC) {
      printf(" ALLOC");
    }
    if (flags & ELF_SECTION_FLAG_EXEC) {
      printf(" EXEC");
    }
    if (flags & ELF_SECTION_FLAG_MERGE) {
      printf(" MERGE");
    }
    if (flags & ELF_SECTION_FLAG_STR) {
      printf(" STR");
    }
    printf(" )\n");

    switch (b32r(&shdr->type)) {
    case ELF_SECTION_TYPE_PROGBITS:
#if 0
      if (flags & ELF_SECTION_FLAG_WRITE)
#endif
      section_set_type(sec, SECTION_TYPE_DATA);
      if (flags & ELF_SECTION_FLAG_EXEC)
	section_set_type(sec, SECTION_TYPE_TEXT);
      if (flags & ELF_SECTION_FLAG_STR)
	section_set_type(sec, SECTION_TYPE_RODATA);
      break;
    case ELF_SECTION_TYPE_NOBITS:
      section_set_type(sec, SECTION_TYPE_BSS);
      break;
    case ELF_SECTION_TYPE_STRTAB:
      section_set_type(sec, SECTION_TYPE_STRING);
      break;
    default:
      break;
    }

    section_set_virtual_addr(sec, b32r(&shdr->address));

    if (flags & ELF_SECTION_FLAG_ALLOC) {
      section_set_memory_size(sec, b32r(&shdr->size));

      /*
       * セクションがどのセグメントに含まれているか検索し，
       * セグメントのPAからセクションのPAを計算する．
       * セクションの virtual_addr, memory_size を参照するので，
       * virtual_addr と memory_size の設定後に行うこと．
       */
      paddr = elf_get_paddr(header,
			    section_get_virtual_addr(sec),
			    section_get_memory_size(sec));
      section_set_physical_addr(sec, paddr);
    }

    if (b32r(&shdr->type) == ELF_SECTION_TYPE_PROGBITS) {
      section_write(sec, b32r(&shdr->size), ((char *)header + b32r(&shdr->offset)));
      section_set_file_size(sec, section_get_size(sec));
    }
  }

  return seclist;
}

/* ELF形式の書き込み用関数群 */

static int elf_delete_waste_section(section_list seclist)
{
  section sec, sec_next;
  int count = 0;

  for (sec = section_list_search(seclist, NULL); sec; sec = sec_next) {
    sec_next = section_get_next(sec);
    if (section_get_memory_size(sec) == 0) {
      section_list_extract(seclist, sec);
      section_destroy(sec);
      count++;
    }
  }

  return count;
}

static section elf_make_null_section(section_list seclist)
{
  section sec;

  sec = section_create("");
  section_set_type(sec, SECTION_TYPE_UNKNOWN);
  section_list_insert(seclist, section_list_search(seclist, NULL), sec);

  return sec;
}

static section elf_make_name_section(section_list seclist)
{
  section sec, shstrsec;
  char *name;

  shstrsec = section_create(".shstrtab");
  section_set_type(shstrsec, SECTION_TYPE_STRING);
  section_list_insert(seclist, NULL, shstrsec);

  for (sec = section_list_search(seclist, NULL); sec; sec = section_get_next(sec)) {
    name = section_get_name(sec);
    section_write(shstrsec, strlen(name) + 1, name);
  }

  section_set_file_size(shstrsec, section_get_size(shstrsec));

  return shstrsec;
}

static int elf_program_header_number(section_list seclist)
{
  section sec;
  int count = 0;

  for (sec = section_list_search(seclist, NULL); sec; sec = section_get_next(sec)) {
    switch (section_get_type(sec)) {
    case SECTION_TYPE_TEXT:
    case SECTION_TYPE_RODATA:
    case SECTION_TYPE_DATA:
    case SECTION_TYPE_BSS:
      count++;
      break;

    case SECTION_TYPE_STRING:
    case SECTION_TYPE_UNKNOWN:
    case SECTION_TYPE_OTHER:
    default:
      continue;
    }
  }

  return count;
}

static long elf_make_program_header(block blk, section_list seclist, long offset)
{
  struct elf_program_header phdr;
  section sec;
  int count = 0;
  long flags, size = 0;

  for (sec = section_list_search(seclist, NULL); sec; sec = section_get_next(sec)) {
    memset(&phdr, 0, sizeof(phdr));

    flags = 0;
    switch (section_get_type(sec)) {
    case SECTION_TYPE_TEXT:
      flags |= ELF_SEGMENT_FLAG_EXEC|ELF_SEGMENT_FLAG_READ;
      break;
    case SECTION_TYPE_RODATA:
      flags |= ELF_SEGMENT_FLAG_READ;
      break;
    case SECTION_TYPE_DATA:
    case SECTION_TYPE_BSS:
      flags |= ELF_SEGMENT_FLAG_WRITE|ELF_SEGMENT_FLAG_READ;
      break; 
    case SECTION_TYPE_STRING:
    case SECTION_TYPE_UNKNOWN:
    case SECTION_TYPE_OTHER:
    default:
      continue;
    }

    b32w(&phdr.type, ELF_SEGMENT_TYPE_LOAD);
    b32w(&phdr.offset, offset + section_list_count_offset(seclist, sec));
    b32w(&phdr.virtual_addr, section_get_virtual_addr(sec));
    b32w(&phdr.physical_addr, section_get_physical_addr(sec));
    b32w(&phdr.file_size, section_get_file_size(sec));
    b32w(&phdr.memory_size, section_get_memory_size(sec));
    b32w(&phdr.flags, flags);
    b32w(&phdr.align, sizeof(long));
    count++;
    size += block_write(blk, sizeof(phdr), (char *)&phdr);
  }

  return count;
}

static long elf_make_sections(block blk, section_list seclist)
{
  section sec;
  long size = 0;

  for (sec = section_list_search(seclist, NULL); sec; sec = section_get_next(sec)) {
    size += block_write(blk, section_get_size(sec), section_get_data(sec));
  }

  return size;
}

static int elf_make_section_header(block blk, section_list seclist, long offset)
{
  struct elf_section_header shdr;
  section sec, shstrsec;
  int n, count = 0;
  long size = 0;
  long flags;

  shstrsec = section_list_search(seclist, ".shstrtab");

  for (sec = section_list_search(seclist, NULL); sec; sec = section_get_next(sec)) {
    memset(&shdr, 0, sizeof(shdr));

    flags = 0;
    switch (section_get_type(sec)) {
    case SECTION_TYPE_TEXT:
      b32w(&shdr.type, ELF_SECTION_TYPE_PROGBITS);
      flags |= ELF_SECTION_FLAG_ALLOC | ELF_SECTION_FLAG_EXEC;
      break;
    case SECTION_TYPE_RODATA:
      b32w(&shdr.type, ELF_SECTION_TYPE_PROGBITS);
      flags |= ELF_SECTION_FLAG_ALLOC | ELF_SECTION_FLAG_MERGE | ELF_SECTION_FLAG_STR;
      break;
    case SECTION_TYPE_DATA:
      b32w(&shdr.type, ELF_SECTION_TYPE_PROGBITS);
      flags |= ELF_SECTION_FLAG_ALLOC | ELF_SECTION_FLAG_WRITE;
      break;
    case SECTION_TYPE_BSS:
      b32w(&shdr.type, ELF_SECTION_TYPE_NOBITS);
      flags |= ELF_SECTION_FLAG_ALLOC | ELF_SECTION_FLAG_WRITE;
      break;
    case SECTION_TYPE_STRING:
      b32w(&shdr.type, ELF_SECTION_TYPE_STRTAB); break;
    case SECTION_TYPE_UNKNOWN:
    case SECTION_TYPE_OTHER:
    default:
      b32w(&shdr.type, ELF_SECTION_TYPE_NULL); break;
    }

    for (n = 0; n < section_get_size(shstrsec);
	 n += strlen(section_get_data(shstrsec) + n) + 1) {
      if (!strcmp(section_get_data(shstrsec) + n, section_get_name(sec)))
	break;
    }

    b32w(&shdr.name, n);

    b32w(&shdr.flags, flags);
    b32w(&shdr.address, section_get_virtual_addr(sec));

    if (section_get_type(sec) == SECTION_TYPE_UNKNOWN)
      b32w(&shdr.offset, 0); /* for NULL section */
    else
      b32w(&shdr.offset, offset + section_list_count_offset(seclist, sec));

    if (section_get_file_size(sec) == 0)
      b32w(&shdr.size, section_get_memory_size(sec)); /* for BSS */
    else
      b32w(&shdr.size, section_get_file_size(sec));

    b32w(&shdr.link, 0);
    b32w(&shdr.info, 0);
    b32w(&shdr.align, sizeof(long));
    b32w(&shdr.entsize, 0);

    count++;
    size += block_write(blk, sizeof(shdr), (char *)&shdr);
  }

  return count;
}

static long elf_make_elf_header(block blk,
				long entry_point,
				long program_header_offset,
				int program_header_num,
				long section_header_offset,
				int section_header_num,
				int section_name_index)
{
  struct elf_header ehdr;

  memset(&ehdr, 0, sizeof(ehdr));
  memcpy(&ehdr.id.magic, "\x7f" "ELF", 4);
  ehdr.id.class       = ELF_CLASS_32BIT;
  ehdr.id.format      = ELF_FORMAT_BIG_ENDIAN;
  ehdr.id.version     = ELF_VERSION_CURRENT;
  ehdr.id.abi         = ELF_ABI_NONE;
  ehdr.id.abi_version = 0;

  b16w(&ehdr.type, ELF_TYPE_EXEC);
  b16w(&ehdr.arch, ELF_ARCH_H8_300);
  b32w(&ehdr.version, 1);
  b32w(&ehdr.entry_point, entry_point);
  b32w(&ehdr.program_header_offset, program_header_offset);
  b32w(&ehdr.section_header_offset, section_header_offset);
  b32w(&ehdr.flags, 0); //0x00810000

  b16w(&ehdr.header_size, sizeof(struct elf_header));
  b16w(&ehdr.program_header_size, sizeof(struct elf_program_header));
  b16w(&ehdr.program_header_num, program_header_num);
  b16w(&ehdr.section_header_size, sizeof(struct elf_section_header));
  b16w(&ehdr.section_header_num, section_header_num);
  b16w(&ehdr.section_name_index, section_name_index);

  return block_write(blk, sizeof(ehdr), (char *)&ehdr);
}

block_list elf_make(section_list seclist)
{
  block_list blklist;
  block blk_elf_header;
  block blk_program_header;
  block blk_sections;
  block blk_section_header;
  int n;
  int program_header_number;
  int section_header_number;
  long section_offset;

  blklist = block_list_create();

  blk_elf_header      = block_create("ELF header");
  blk_program_header  = block_create("program header");
  blk_sections        = block_create("sections");
  blk_section_header  = block_create("section header");

  block_list_insert(blklist, NULL, blk_elf_header);
  block_list_insert(blklist, NULL, blk_program_header);
  block_list_insert(blklist, NULL, blk_sections);
  block_list_insert(blklist, NULL, blk_section_header);

  /* 不要なセクション情報を削除 */
  elf_delete_waste_section(seclist);

  /* 先頭にNULLセクションを追加 */
  elf_make_null_section(seclist);

  /* セクション名格納用セクション(.shstrtab)の作成 */
  elf_make_name_section(seclist);
  section_list_align(seclist, sizeof(long));

  /* プログラムヘッダを作成 */
  n = elf_program_header_number(seclist);
  section_offset = sizeof(struct elf_header) + sizeof(struct elf_program_header) * n;
  program_header_number = elf_make_program_header(blk_program_header, seclist, section_offset);
  if (program_header_number != n) {
    fprintf(stderr, "Fail to make program header.\n");
    return NULL;
  }

  /* セクションの本体を作成 */
  elf_make_sections(blk_sections, seclist);

  /* セクションヘッダを作成 */
  section_header_number = elf_make_section_header(blk_section_header, seclist, section_offset);

  block_list_align(blklist, sizeof(long));

  elf_make_elf_header(blk_elf_header,
		      section_list_get_entry_point(seclist),
		      sizeof(struct elf_header) + 
		      block_list_count_offset(blklist, blk_program_header),
		      program_header_number,
		      sizeof(struct elf_header) + 
		      block_list_count_offset(blklist, blk_section_header),
		      section_header_number,
		      section_list_count_position(seclist, section_list_search(seclist, ".shstrtab")));

  return blklist;
}
