#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include "coff.h"
#include "elf.h"
#include "section.h"
#include "block.h"
#include "argument.h"

static void usage()
{
  printf("usage {-h|-help}\n");
  printf("usage -from <format> -to <format> -i <filename> -o <filename>\n");
  exit(0);
}

static char *fromstr = NULL;
static char *tostr   = NULL;
static char *infile  = NULL;
static char *outfile = NULL;

static const Argument args[] = {
  {"-h",      ARGUMENT_TYPE_FUNCTION, usage},
  {"-help",   ARGUMENT_TYPE_FUNCTION, usage},
  {"-f",      ARGUMENT_TYPE_STRING,   &fromstr},
  {"-from",   ARGUMENT_TYPE_STRING,   &fromstr},
  {"-t" ,     ARGUMENT_TYPE_STRING,   &tostr},
  {"-to",     ARGUMENT_TYPE_STRING,   &tostr},
  {"-i",      ARGUMENT_TYPE_STRING,   &infile},
  {"-in",     ARGUMENT_TYPE_STRING,   &infile},
  {"-input",  ARGUMENT_TYPE_STRING,   &infile},
  {"-o",      ARGUMENT_TYPE_STRING,   &outfile},
  {"-out",    ARGUMENT_TYPE_STRING,   &outfile},
  {"-output", ARGUMENT_TYPE_STRING,   &outfile},
  {NULL,      ARGUMENT_TYPE_NONE,     NULL}
};

typedef enum {
  OBJECT_FORMAT_UNKNOWN = 0,
  OBJECT_FORMAT_ELF,
  OBJECT_FORMAT_COFF,
} object_format_t;

char *object_format_name[] = {
  "unknown",
  "elf",
  "coff",
  NULL
};

object_format_t str2format(char *s)
{
  int i;
  if (s) {
    for (i = 0; object_format_name[i]; i++) {
      if (!strcmp(s, object_format_name[i]))
	return i;
    }
  }
  return OBJECT_FORMAT_UNKNOWN;
}

int main(int argc, char *argv[])
{
  object_format_t from, to;
  char *head;
  int rfd, size;
  FILE *wfp;
  struct stat sb;
  section_list seclist;
  block_list blklist;

  argument_read(&argc, argv, args);

  from = str2format(fromstr);
  to   = str2format(tostr);

  if (!infile || !outfile)
    usage();

  if ((from == OBJECT_FORMAT_UNKNOWN) || (to == OBJECT_FORMAT_UNKNOWN))
    usage();

  rfd = open(infile, O_RDONLY);
  if (rfd < 0) {
    fprintf(stderr, "file not found. (%s)\n", infile);
    exit(-1);
  }

  fstat(rfd, &sb);
  size = sb.st_size;

  head = mmap(NULL, size, PROT_READ, MAP_SHARED, rfd, 0);

  switch (from) {
  case OBJECT_FORMAT_ELF:
    seclist = elf_read(head);
    break;
  case OBJECT_FORMAT_COFF:
    seclist = coff_read(head);
    break;
  case OBJECT_FORMAT_UNKNOWN:
  default:
    fprintf(stderr, "input format is invalid.\n");
    exit(-1);
  }

  section_list_print(seclist, stdout);

  switch (to) {
  case OBJECT_FORMAT_ELF:
    blklist = elf_make(seclist);
    break;
  case OBJECT_FORMAT_COFF:
    blklist = coff_make(seclist);
    break;
  case OBJECT_FORMAT_UNKNOWN:
  default:
    fprintf(stderr, "output format is invalid.\n");
    exit(-1);
  }

  printf("Convert succeeded.\n");

  wfp = fopen(outfile, "w");
  if (wfp == NULL) {
    fprintf(stderr, "cannot create file. (%s)\n", outfile);
    exit(-1);
  }
  block_list_out(blklist, wfp);

  printf("Finished.\n");

  fclose(wfp);

  munmap(head, size);
  close(rfd);

  return 0;
}
