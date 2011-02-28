#include <stdlib.h>
#include <string.h>

#include "section.h"
#include "lib.h"

struct _section_list {
  struct _section *head;
  struct _section **tail;
  int length;

  struct {
    section_list_type_t type;
    long entry_point;
  } param;
};

struct _section {
  struct _section *next;
  char *name;
  long size;
  long offset;
  long datasize;
  char *data;

  struct {
    section_type_t type;
    long physical_addr;
    long virtual_addr;
    long file_size;
    long memory_size;
  } param;
};

/*
 * for section_list
 */

section_list section_list_create()
{
  section_list seclist;

  seclist = malloc(sizeof(*seclist));
  memset(seclist, 0, sizeof(*seclist));

  seclist->head = NULL;
  seclist->tail = &seclist->head;
  seclist->length = 0;

  seclist->param.type = SECTION_LIST_TYPE_UNKNOWN;
  seclist->param.entry_point = 0;

  return seclist;
}

section_list section_list_destroy(section_list seclist)
{
  section sec;

  while (seclist->head) {
    sec = seclist->head;
    section_list_extract(seclist, sec);
    section_destroy(sec);
  }

  memset(seclist, 0, sizeof(*seclist));
  free(seclist);

  return NULL;
}

section section_list_insert(section_list seclist, section place, section sec)
{
  section *secp;

  for (secp = &seclist->head; *secp; secp = &(*secp)->next)
    if (*secp == place)
      break;

  if (*secp == place) {
    sec->next = *secp;
    *secp = sec;
    if (seclist->tail == secp)
      seclist->tail = &sec->next;
    seclist->length++;
    return sec;
  }

  return NULL;
}

section section_list_extract(section_list seclist, section sec)
{
  section *secp;

  for (secp = &seclist->head; *secp; secp = &(*secp)->next) {
    if (*secp == sec) {
      *secp = sec->next;
      sec->next = NULL;
      if (seclist->tail == &sec->next)
	seclist->tail = secp;
      seclist->length--;
      return sec;
    }
  }

  return NULL;
}

section section_list_search(section_list seclist, char *name)
{
  section sec;

  if (!name) return seclist->head;

  for (sec = seclist->head; sec; sec = sec->next) {
    if (!strcmp(sec->name, name))
      return sec;
  }

  return NULL;
}

int section_list_count_position(section_list seclist, section sec)
{
  section sec2;
  int position = 0;
  for (sec2 = seclist->head; sec2; sec2 = sec2->next) {
    if (sec2 == sec)
      return position;
    position++;
  }
  return -1;
}

long section_list_count_offset(section_list seclist, section sec)
{
  section sec2;
  long offset = 0;
  for (sec2 = seclist->head; sec2; sec2 = sec2->next) {
    if (sec2 == sec)
      return offset;
    offset += sec2->size;
  }
  return -1;
}

int section_list_align(section_list seclist, int size)
{
  section sec;
  for (sec = seclist->head; sec; sec = sec->next) {
    section_align(sec, size);
  }
  return 0;
}

int section_list_get_length(section_list seclist)
{
  return seclist->length;
}

section_list_type_t section_list_get_type(section_list seclist)
{
  return seclist->param.type;
}

section_list_type_t section_list_set_type(section_list seclist,
					  section_list_type_t type)
{
  return seclist->param.type = type;
}

long section_list_get_entry_point(section_list seclist)
{
  return seclist->param.entry_point;
}

long section_list_set_entry_point(section_list seclist, long entry_point)
{
  return seclist->param.entry_point = entry_point;
}

int section_list_print(section_list seclist, FILE *out)
{
  char *s;
  section sec;

  fprintf(out, "----\nSection list information.\n");

  s = "(NULL)";
  switch (seclist->param.type) {
  case SECTION_LIST_TYPE_UNKNOWN:  s = "UNKNOWN";  break;
  case SECTION_LIST_TYPE_RELOCATE: s = "RELOCATE"; break;
  case SECTION_LIST_TYPE_EXEC:     s = "EXEC";     break;
  default:                         s = "UNKNOWN";  break;
  }
  fprintf(out, "type       :%s\n", s);
  fprintf(out, "length     :%d\n", seclist->length);
  fprintf(out, "entry point:0x%08lx\n", seclist->param.entry_point);

  fprintf(out, "\nSection information.\n");
  for (sec = seclist->head; sec; sec = sec->next) {
    fprintf(out, "\n");
    section_print(sec, out);
  }
  fprintf(out, "----\n");

  return 0;
}

/*
 * for section
 */

section section_create(char *name)
{
  section sec;

  sec = malloc(sizeof(*sec));
  memset(sec, 0, sizeof(*sec));

  sec->next = NULL;
  sec->name = strdup(name);
  sec->size = 0;
  sec->offset = 0;
  sec->datasize = 1024;
  sec->data = malloc(sec->datasize);

  sec->param.type = SECTION_TYPE_UNKNOWN;
  sec->param.physical_addr = 0;
  sec->param.virtual_addr = 0;
  sec->param.memory_size = 0;

  return sec;
}

section section_create_by_copy(section sec)
{
  section newsec;

  newsec = section_create(sec->name);
  section_write(newsec, sec->size, sec->data);

  return newsec;
}

section section_destroy(section sec)
{
  if (sec->name) free(sec->name);
  if (sec->data) free(sec->data);

  memset(sec, 0, sizeof(*sec));
  free(sec);

  return NULL;
}

static long section_extend(section sec, long size)
{
  char *newbuf;
  if (size > sec->datasize) {
    newbuf = malloc(size);
    memset(newbuf, 0, size);
    if (sec->data) {
      memcpy(newbuf, sec->data, sec->datasize);
      free(sec->data);
    }
    sec->datasize = size;
    sec->data = newbuf;
  }
  return size;
}

long section_seek(section sec, long offset)
{
  section_extend(sec, offset);
  sec->offset = offset;
  return offset;
}

long section_read(section sec, long size, char *data)
{
  if (sec->offset > sec->size)
    return 0;
  if (sec->offset + size > sec->size)
    size = sec->size - sec->offset;
  memcpy(data, sec->data + sec->offset, size);
  sec->offset += size;
  return size;
}

long section_write(section sec, long size, char *data)
{
  section_extend(sec, sec->offset + size);
  if (data)
    memcpy(sec->data + sec->offset, data, size);
  else
    memset(sec->data + sec->offset, 0, size);
  sec->offset += size;
  if (sec->offset > sec->size)
    sec->size = sec->offset;
  return size;
}

int section_align(section sec, int size)
{
  int align;
  align = sec->size % size;
  if (align) section_write(sec, size - align, NULL);
  return align;
}

section section_get_next(section sec)
{
  return sec->next;
}

char *section_get_name(section sec)
{
  return sec->name;
}

long section_get_size(section sec)
{
  return sec->size;
}

char *section_get_data(section sec)
{
  return sec->data;
}

section_type_t section_get_type(section sec)
{
  return sec->param.type;
}

section_type_t section_set_type(section sec, section_type_t type)
{
  return sec->param.type = type;
}

long section_get_physical_addr(section sec)
{
  return sec->param.physical_addr;
}

long section_set_physical_addr(section sec, long physical_addr)
{
  return sec->param.physical_addr = physical_addr;
}

long section_get_virtual_addr(section sec)
{
  return sec->param.virtual_addr;
}

long section_set_virtual_addr(section sec, long virtual_addr)
{
  return sec->param.virtual_addr = virtual_addr;
}

long section_get_file_size(section sec)
{
  return sec->param.file_size;
}

long section_set_file_size(section sec, long file_size)
{
  return sec->param.file_size = file_size;
}

long section_get_memory_size(section sec)
{
  return sec->param.memory_size;
}

long section_set_memory_size(section sec, long memory_size)
{
  return sec->param.memory_size = memory_size;
}

int section_print(section sec, FILE *out)
{
  char *s;

  fprintf(out, "%s (", sec->name[0] ? sec->name : "(NULL)");

  s = "(NULL)";
  switch (sec->param.type) {
  case SECTION_TYPE_UNKNOWN: s = "UNKNOWN"; break;
  case SECTION_TYPE_TEXT:    s = "TEXT";    break;
  case SECTION_TYPE_RODATA:  s = "RODATA";  break;
  case SECTION_TYPE_DATA:    s = "DATA";    break;
  case SECTION_TYPE_BSS:     s = "BSS";     break;
  case SECTION_TYPE_STRING:  s = "STRING";  break;
  case SECTION_TYPE_OTHER:   s = "OTHER";   break;
  default:                   s = "UNKNOWN"; break;
  }
  fprintf(out, "%s)\n", s);

  fprintf(out, "size         :0x%08lx  ", sec->size);
  fprintf(out, "file size    :0x%08lx  ", sec->param.file_size);
  fprintf(out, "memory size  :0x%08lx\n", sec->param.memory_size);
  fprintf(out, "physical addr:0x%08lx  ", sec->param.physical_addr);
  fprintf(out, "virtual addr :0x%08lx\n", sec->param.virtual_addr);

  return 0;
}
