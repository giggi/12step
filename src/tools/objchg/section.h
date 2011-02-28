#ifndef _SECTION_H_INCLUDED_
#define _SECTION_H_INCLUDED_

#include <stdio.h>

typedef struct _section_list *section_list;
typedef struct _section *section;

typedef enum {
  SECTION_LIST_TYPE_UNKNOWN = 0,
  SECTION_LIST_TYPE_RELOCATE,
  SECTION_LIST_TYPE_EXEC,
} section_list_type_t;

typedef enum {
  SECTION_TYPE_UNKNOWN = 0,
  SECTION_TYPE_TEXT,
  SECTION_TYPE_RODATA,
  SECTION_TYPE_DATA,
  SECTION_TYPE_BSS,
  SECTION_TYPE_STRING,
  SECTION_TYPE_OTHER,
} section_type_t;

section_list section_list_create();
section_list section_list_destroy(section_list seclist);
section section_list_insert(section_list seclist, section place, section sec);
section section_list_extract(section_list seclist, section sec);
section section_list_search(section_list seclist, char *name);
int section_list_count_position(section_list seclist, section sec);
long section_list_count_offset(section_list seclist, section sec);
int section_list_align(section_list seclist, int size);
int section_list_get_length(section_list seclist);
section_list_type_t section_list_get_type(section_list seclist);
section_list_type_t section_list_set_type(section_list seclist,
					  section_list_type_t type);
long section_list_get_entry_point(section_list seclist);
long section_list_set_entry_point(section_list seclist, long entry_point);
int section_list_print(section_list seclist, FILE *out);

section section_create(char *name);
section section_create_by_copy(section sec);
section section_destroy(section sec);
long section_seek(section sec, long offset);
long section_read(section sec, long size, char *data);
long section_write(section sec, long size, char *data);
int section_align(section sec, int size);
section section_get_next(section sec);
char *section_get_name(section sec);
long section_get_size(section sec);
char *section_get_data(section sec);
section_type_t section_get_type(section sec);
section_type_t section_set_type(section sec, section_type_t type);
long section_get_physical_addr(section sec);
long section_set_physical_addr(section sec, long physical_addr);
long section_get_virtual_addr(section sec);
long section_set_virtual_addr(section sec, long virtual_addr);
long section_get_file_size(section sec);
long section_set_file_size(section sec, long file_size);
long section_get_memory_size(section sec);
long section_set_memory_size(section sec, long memory_size);
int section_print(section sec, FILE *out);

#endif
