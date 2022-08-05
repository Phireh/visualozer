TIME=/usr/bin/time --format="Step took %Es"

CC=clang
CFLAGS= -std=c11 -g

LD=clang
LDFLAGS=-lpthread -lm
LDFLAGS+=`pkg-config --libs glfw3 glew`

GENPCH_FLAGS=-x c-header -fpch-codegen -fpch-debuginfo
USEPCH_FLAGS=-include-pch $(SRCDIR)/main.h.pch

SRCDIR=src

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S), Linux) # flags for Linux build
	LDFLAGS+=-ldl
endif
ifeq ($(UNAME_S), Darwin) # flags for the Mac build
	LDFLAGS+=-framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo -L/usr/local/lib
endif

visualozer: $(SRCDIR)/main.h.pch.o $(SRCDIR)/main.o
	@echo Linking...
	$(TIME) $(LD) $(CFLAGS) $(SRCDIR)/main.h.pch.o $(SRCDIR)/main.o -o visualozer $(LDFLAGS)

$(SRCDIR)/main.o: $(SRCDIR)/main.h.pch $(SRCDIR)/main.c
	@echo Compiling object...
	$(TIME) $(CC) $(USEPCH_FLAGS) $(SRCDIR)/main.c -c -o $(SRCDIR)/main.o $(CFLAGS)

$(SRCDIR)/main.h.pch.o: $(SRCDIR)/main.h.pch
	@echo Compiling PCH...
	$(TIME) $(CC) -c -o $(SRCDIR)/main.h.pch.o $(CFLAGS) $(SRCDIR)/main.h.pch

$(SRCDIR)/main.h.pch: $(SRCDIR)/main.h $(SRCDIR)/nuklear.h $(SRCDIR)/nuklear_glfw_gl3.h $(SRCDIR)/miniaudio.h
	@echo Generating PCH...
	$(TIME) $(CC) $(GENPCH_FLAGS) $(SRCDIR)/main.h $(CFLAGS) -o $(SRCDIR)/main.h.pch

clean:
	rm visualozer src/*.o src/*.pch
