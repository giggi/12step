#ifndef _BLOCK_H_INCLUDED_
#define _BLOCK_H_INCLUDED_

#include <stdio.h>

typedef struct _block_list *block_list;
typedef struct _block *block;

block_list block_list_create();
block_list block_list_destroy(block_list blklist);
block block_list_insert(block_list blklist, block place, block blk);
block block_list_extract(block_list blklist, block blk);
block block_list_search(block_list blklist, char *name);
int block_list_count_position(block_list blklist, block blk);
long block_list_count_offset(block_list blklist, block blk);
int block_list_align(block_list blklist, int size);
long block_list_out(block_list blklist, FILE *out);
int block_list_get_length(block_list blklist);

block block_create(char *name);
block block_destroy(block blk);
long block_seek(block blk, long offset);
long block_read(block blk, long size, char *data);
long block_write(block blk, long size, char *data);
int block_align(block blk, int size);
block block_get_next(block blk);
char *block_get_name(block blk);
long block_get_size(block blk);
char *block_get_data(block blk);
int block_out(block blk, FILE *out);

#endif
