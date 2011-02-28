PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

CC ?= gcc

OBJS = main.o argument.o coff.o elf.o section.o block.o lib.o

TARGET = objchg

CFLAGS = -Wall
#CFLAGS += -g
CFLAGS += -O
LFLAGS =

.SUFFIXES: .c .o

all :		$(TARGET)

$(TARGET) :	$(OBJS)
		$(CC) $(OBJS) -o $(TARGET) $(CFLAGS) $(LFLAGS)

.c.o :		$<
		$(CC) -c $(CFLAGS) $<

install :	$(TARGET)
		cp $(TARGET) $(BINDIR)

uninstall :
		rm -f $(BINDIR)/$(TARGET)

clean :
		rm -f $(OBJS) $(TARGET)
