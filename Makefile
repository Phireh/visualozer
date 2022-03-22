CC=gcc
CFLAGS=-Wall -Wextra -g -pedantic -std=c11 -Og
LDFLAGS=-lpthread -lm
LDFLAGS+=`pkg-config --libs glfw3 glew`
SRCDIR=src


UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux) # flags for Linux build
	LDFLAGS+=-ldl
endif
ifeq ($(UNAME_S), Darwin) # flags for the Mac build
	LDFLAGS+=-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -L/usr/local/lib
endif

visualozer: $(SRCDIR)/main.c $(SRCDIR)/main.h.gch
	$(CC) $(SRCDIR)/main.c $(CFLAGS) -o visualozer $(LDFLAGS)


$(SRCDIR)/main.h.gch: $(SRCDIR)/main.h $(SRCDIR)/nuklear.h $(SRCDIR)/nuklear_glfw_gl3.h $(SRCDIR)/miniaudio.h
	$(CC) $(SRCDIR)/main.h $(CFLAGS) -c -o $(SRCDIR)/main.h.gch $(LDFLAGS)

clean:
	rm visualozer
