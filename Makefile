CC=gcc
CFLAGS=-Wall -Wextra -g -pedantic -std=c11
LDFLAGS=-lpthread -lm
SRCDIR=src


UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux) # flags for Linux build
	LDFLAGS+=-ldl
endif

visualozer: $(SRCDIR)/main.c
	$(CC) $(CFLAGS) $(LDFLAGS) $(SRCDIR)/main.c -o visualozer

clean:
	rm visualozer
