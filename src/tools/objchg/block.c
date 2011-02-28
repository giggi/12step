#include <stdlib.h>
#include <string.h>

#include "block.h"

struct _block_list {
  struct _block *head;
  struct _block **tail;
  int length;
};

struct _block {
  struct _block *next;
  char *name;
  long size;
  long offset;
  long datasize;
  char *data;
};

/*
 * for block_list
 */

block_list block_list_create()
{
  block_list blklist;

  blklist = malloc(sizeof(*blklist));
  memset(blklist, 0, sizeof(*blklist));

  blklist->head = NULL;
  blklist->tail = &blklist->head;
  blklist->length = 0;

  return blklist;
}

block_list block_list_destroy(block_list blklist)
{
  block blk;

  while (blklist->head) {
    blk = blklist->head;
    block_list_extract(blklist, blk);
    block_destroy(blk);
  }

  memset(blklist, 0, sizeof(*blklist));
  free(blklist);

  return NULL;
}

block block_list_insert(block_list blklist, block place, block blk)
{
  block *blkp;

  for (blkp = &blklist->head; *blkp; blkp = &(*blkp)->next)
    if (*blkp == place)
      break;

  if (*blkp == place) {
    blk->next = *blkp;
    *blkp = blk;
    if (blklist->tail == blkp)
      blklist->tail = &blk->next;
    blklist->length++;
    return blk;
  }

  return NULL;
}

block block_list_extract(block_list blklist, block blk)
{
  block *blkp;

  for (blkp = &blklist->head; *blkp; blkp = &(*blkp)->next) {
    if (*blkp == blk) {
      *blkp = blk->next;
      blk->next = NULL;
      if (blklist->tail == &blk->next)
	blklist->tail = blkp;
      blklist->length--;
      return blk;
    }
  }

  return NULL;
}

block block_list_search(block_list blklist, char *name)
{
  block blk;

  if (!name) return blklist->head;

  for (blk = blklist->head; blk; blk = blk->next) {
    if (!strcmp(blk->name, name))
      return blk;
  }

  return NULL;
}

int block_list_count_position(block_list blklist, block blk)
{
  block blk2;
  int position = 0;
  for (blk2 = blklist->head; blk2; blk2 = blk2->next) {
    if (blk2 == blk)
      return position;
    position++;
  }
  return -1;
}

long block_list_count_offset(block_list blklist, block blk)
{
  block blk2;
  long offset = 0;
  for (blk2 = blklist->head; blk2; blk2 = blk2->next) {
    if (blk2 == blk)
      return offset;
    offset += blk2->size;
  }
  return -1;
}

int block_list_align(block_list blklist, int size)
{
  block blk;
  for (blk = blklist->head; blk; blk = blk->next) {
    block_align(blk, size);
  }
  return 0;
}

long block_list_out(block_list blklist, FILE *out)
{
  block blk;
  long size = 0;

  for (blk = blklist->head; blk; blk = blk->next) {
    size += block_out(blk, out);
  }

  return size;
}

int block_list_get_length(block_list blklist)
{
  return blklist->length;
}

/*
 * for block
 */

block block_create(char *name)
{
  block blk;

  blk = malloc(sizeof(*blk));
  memset(blk, 0, sizeof(*blk));

  blk->next = NULL;
  blk->name = strdup(name);
  blk->size = 0;
  blk->offset = 0;
  blk->datasize = 1024;
  blk->data = malloc(blk->datasize);

  return blk;
}

block block_destroy(block blk)
{
  if (blk->name) free(blk->name);
  if (blk->data) free(blk->data);

  memset(blk, 0, sizeof(*blk));
  free(blk);

  return NULL;
}

static long block_extend(block blk, long size)
{
  char *newbuf;
  if (size > blk->datasize) {
    newbuf = malloc(size);
    memset(newbuf, 0, size);
    if (blk->data) {
      memcpy(newbuf, blk->data, blk->datasize);
      free(blk->data);
    }
    blk->datasize = size;
    blk->data = newbuf;
  }
  return size;
}

long block_seek(block blk, long offset)
{
  block_extend(blk, offset);
  blk->offset = offset;
  return offset;
}

long block_read(block blk, long size, char *data)
{
  if (blk->offset > blk->size)
    return 0;
  if (blk->offset + size > blk->size)
    size = blk->size - blk->offset;
  memcpy(data, blk->data + blk->offset, size);
  blk->offset += size;
  return size;
}

long block_write(block blk, long size, char *data)
{
  block_extend(blk, blk->offset + size);
  if (data)
    memcpy(blk->data + blk->offset, data, size);
  else
    memset(blk->data + blk->offset, 0, size);
  blk->offset += size;
  if (blk->offset > blk->size)
    blk->size = blk->offset;
  return size;
}

int block_align(block blk, int size)
{
  int align;
  align = blk->size % size;
  if (align) block_write(blk, size - align, NULL);
  return align;
}

block block_get_next(block blk)
{
  return blk->next;
}

char *block_get_name(block blk)
{
  return blk->name;
}

long block_get_size(block blk)
{
  return blk->size;
}

char *block_get_data(block blk)
{
  return blk->data;
}

int block_out(block blk, FILE *out)
{
  return fwrite(blk->data, blk->size, 1, out);
}
