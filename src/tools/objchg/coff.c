#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "coff.h"
#include "section.h"
#include "lib.h"

struct coff_header {
  short magic;
  short section_num;
  long time;
  long symbol_table;
  long symbol_num;
  short optional_header_size;
  short flags;
#define COFF_HDR_FLAG_REL     0x0001
#define COFF_HDR_FLAG_EXEC    0x0002
#define COFF_HDR_FLAG_NOLNNUM 0x0004
#define COFF_HDR_FLAG_NOSYM   0x0008
};

struct coff_optional_header {
  short magic;
  short version;
  long text_size; /* .textのサイズ */
  long data_size; /* .dataのサイズ */
  long bss_size;  /* .bssのサイズ */
  long entry_point;
  long text_offset; /* .textのVA */
  long data_offset; /* .dataのVA */
};

struct coff_section_header {
  char name[8];
  long physical_addr;
  long virtual_addr;
  long size;
  long offset;
  long relocation;
  long line_number;
  short relocation_num;
  short line_number_num;
  long  flags;
#define COFF_SHDR_FLAG_TEXT 0x0020
#define COFF_SHDR_FLAG_DATA 0x0040
#define COFF_SHDR_FLAG_BSS  0x0080
};

/* COFF形式の読み込み用関数群 */

section_list coff_read(char *buf)
{
  section_list seclist;
  section sec;
  struct coff_header *header = (struct coff_header *)buf;
  struct coff_optional_header *ohdr;
  struct coff_section_header *shdr;
  char name[sizeof(shdr->name) + 1];
  int i, num;
  short sflags;
  long lflags;

  if (b16r(&header->magic) == 0x8301) {
    printf("This is COFF file.\n");
  } else {
    printf("This is not COFF file.\n");
    return NULL;
  }

  seclist = section_list_create();

  printf("\nCOFF header information.\n");

  printf("flags:");
  sflags = b16r(&header->flags);
  if (sflags & COFF_HDR_FLAG_REL) {
    printf(" REL");
    section_list_set_type(seclist, SECTION_LIST_TYPE_RELOCATE);
  }
  if (sflags & COFF_HDR_FLAG_EXEC) {
    printf(" EXEC");
    section_list_set_type(seclist, SECTION_LIST_TYPE_EXEC);
  }
  if (sflags & COFF_HDR_FLAG_NOLNNUM) {
    printf(" NOLNNUM");
  }
  if (sflags & COFF_HDR_FLAG_NOSYM) {
    printf(" NOSYM");
  }
  printf("\n");

  printf("\nOptional header information.\n");
  ohdr = (struct coff_optional_header *)
    ((char *)header + sizeof(struct coff_header));
  printf("magic       : %s\n",  v2ss(b16r(&ohdr->magic)));
  printf("version     : %s\n",  v2ss(b16r(&ohdr->version)));
  printf("text size   : %s\n", v2sl(b32r(&ohdr->text_size)));
  printf("data size   : %s\n", v2sl(b32r(&ohdr->data_size)));
  printf("BSS size    : %s\n", v2sl(b32r(&ohdr->bss_size)));
  printf("entry point : %s\n", v2sl(b32r(&ohdr->entry_point)));
  printf("text offset : %s\n", v2sl(b32r(&ohdr->text_offset)));
  printf("data offset : %s\n", v2sl(b32r(&ohdr->data_offset)));

  section_list_set_entry_point(seclist, b32r(&ohdr->entry_point));

  printf("\nSection header information.\n");
  num = b16r(&header->section_num);
  for (i = 0; i < num; i++) {
    shdr = (struct coff_section_header *)
      ((char *)header + sizeof(struct coff_header) +
       b16r(&header->optional_header_size) +
      sizeof(struct coff_section_header) * i);

    strncpy(name, shdr->name, sizeof(name));
    sec = section_create(name);
    section_list_insert(seclist, NULL, sec);

    printf("\n");
    printf("%s\n", name);
    printf("paddr  :0x%08lx  ",    b32r(&shdr->physical_addr));
    printf("vaddr  :0x%08lx  ",    b32r(&shdr->virtual_addr));
    printf("size   :0x%08lx  ",    b32r(&shdr->size));
    printf("offset :0x%08lx\n",    b32r(&shdr->offset));
    printf("reloc  :0x%08lx  ",    b32r(&shdr->relocation));
    printf("lnnum  :0x%08lx  ",    b32r(&shdr->line_number));
    printf("nreloc :0x%04x      ", b16r(&shdr->relocation_num));
    printf("nlnnum :0x%04x\n",     b16r(&shdr->line_number_num));

    section_set_physical_addr(sec, b32r(&shdr->physical_addr));
    section_set_virtual_addr(sec, b32r(&shdr->virtual_addr));

    lflags = b32r(&shdr->flags);
    printf("flags  :0x%08lx (", lflags);
    section_set_type(sec, SECTION_TYPE_OTHER);
    if (lflags & COFF_SHDR_FLAG_TEXT) {
      printf(" TEXT");
      if (!strcmp(name, ".rodata"))
	section_set_type(sec, SECTION_TYPE_RODATA);
      else
	section_set_type(sec, SECTION_TYPE_TEXT);
    }
    if (lflags & COFF_SHDR_FLAG_DATA) {
      printf(" DATA");
      section_set_type(sec, SECTION_TYPE_DATA);
    }
    if (lflags & COFF_SHDR_FLAG_BSS) {
      printf(" BSS");
      section_set_type(sec, SECTION_TYPE_BSS);
    }
    printf(" )\n");

    section_set_memory_size(sec, b32r(&shdr->size));

    if (b32r(&shdr->offset)) {
      section_write(sec, b32r(&shdr->size), ((char *)header + b32r(&shdr->offset)));
      section_set_file_size(sec, section_get_size(sec));
    }
  }

  return seclist;
}

/* COFF形式の書き込み用関数群 */

static int coff_delete_waste_section(section_list seclist)
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

static long coff_make_coff_header(block blk, int section_num)
{
  struct coff_header chdr;

  memset(&chdr, 0, sizeof(chdr));
  b16w(&chdr.magic, 0x8301);
  b16w(&chdr.section_num, section_num);
  b16w(&chdr.optional_header_size, sizeof(struct coff_optional_header));
  b16w(&chdr.flags, COFF_HDR_FLAG_EXEC);

  return block_write(blk, sizeof(chdr), (char *)&chdr);
}

static long coff_make_optional_header(block blk,
				      long text_size,
				      long data_size,
				      long bss_size,
				      long entry_point,
				      long text_offset,
				      long data_offset)
{
  struct coff_optional_header ohdr;

  memset(&ohdr, 0, sizeof(ohdr));
  b32w(&ohdr.text_size, text_size);
  b32w(&ohdr.data_size, data_size);
  b32w(&ohdr.bss_size, bss_size);
  b32w(&ohdr.entry_point, entry_point);
  b32w(&ohdr.text_offset, text_offset);
  b32w(&ohdr.data_offset, data_offset);

  return block_write(blk, sizeof(ohdr), (char *)&ohdr);
}

static int coff_make_section_header(block blk, section_list seclist, long offset)
{
  struct coff_section_header shdr;
  section sec;
  int count = 0;
  long size = 0;
  long lflags;

  for (sec = section_list_search(seclist, NULL); sec; sec = section_get_next(sec)) {
    memset(&shdr, 0, sizeof(shdr));

    strncpy(shdr.name, section_get_name(sec), sizeof(shdr.name));

    b32w(&shdr.physical_addr, section_get_physical_addr(sec));
    b32w(&shdr.virtual_addr, section_get_virtual_addr(sec));
    b32w(&shdr.size, section_get_memory_size(sec));
    if (section_get_file_size(sec))
      b32w(&shdr.offset, offset + section_list_count_offset(seclist, sec));
    else
      b32w(&shdr.offset, 0);
    b32w(&shdr.relocation, 0);
    b32w(&shdr.line_number, 0);
    b16w(&shdr.relocation_num, 0);
    b16w(&shdr.line_number_num, 0);

    lflags = 0;
    switch (section_get_type(sec)) {
    case SECTION_TYPE_TEXT:   /* fall through */
    case SECTION_TYPE_RODATA: lflags |= COFF_SHDR_FLAG_TEXT; break;
    case SECTION_TYPE_DATA:   lflags |= COFF_SHDR_FLAG_DATA; break;
    case SECTION_TYPE_BSS:    lflags |= COFF_SHDR_FLAG_BSS; break;
    case SECTION_TYPE_STRING: /* fall through */
    case SECTION_TYPE_OTHER:  /* fall through */
    default:                  break;
    }
    b32w(&shdr.flags, lflags);

    count++;
    size += block_write(blk, sizeof(shdr), (char *)&shdr);
  }

  return count;
}

static long coff_make_sections(block blk, section_list seclist)
{
  section sec;
  long size = 0;

  for (sec = section_list_search(seclist, NULL); sec; sec = section_get_next(sec)) {
    size += block_write(blk, section_get_size(sec), section_get_data(sec));
  }

  return size;
}

block_list coff_make(section_list seclist)
{
  block_list blklist;
  block blk_coff_header;
  block blk_optional_header;
  block blk_section_header;
  block blk_sections;
  section text_sec, data_sec, bss_sec;
  int section_header_number;

  blklist = block_list_create();

  blk_coff_header     = block_create("COFF header");
  blk_optional_header = block_create("optional header");
  blk_section_header  = block_create("section header");
  blk_sections        = block_create("sections");

  block_list_insert(blklist, NULL, blk_coff_header);
  block_list_insert(blklist, NULL, blk_optional_header);
  block_list_insert(blklist, NULL, blk_section_header);
  block_list_insert(blklist, NULL, blk_sections);

  section_list_align(seclist, sizeof(long));

  text_sec = section_list_search(seclist, ".text");
  data_sec = section_list_search(seclist, ".data");
  bss_sec  = section_list_search(seclist, ".bss");

  /* 不要なセクション情報を削除 */
  coff_delete_waste_section(seclist);
  section_header_number = section_list_get_length(seclist);

  /* COFFヘッダを作成 */
  coff_make_coff_header(blk_coff_header, section_list_get_length(seclist));

  /* オプションヘッダを作成 */
  coff_make_optional_header(blk_optional_header,
			    section_get_memory_size(text_sec),
			    section_get_memory_size(data_sec),
			    section_get_memory_size(bss_sec),
			    section_list_get_entry_point(seclist),
			    section_get_virtual_addr(text_sec),
			    section_get_virtual_addr(data_sec));

  /* セクションヘッダを作成 */
  coff_make_section_header(blk_section_header, seclist,
			   sizeof(struct coff_header) + sizeof(struct coff_optional_header) + sizeof(struct coff_section_header) * section_header_number);

  /* セクションの本体を作成 */
  coff_make_sections(blk_sections, seclist);

  block_list_align(blklist, sizeof(long));

  return blklist;
}
