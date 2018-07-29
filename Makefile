PROJECT := BitDeque
SOURCES := $(notdir $(shell find . -follow -name '*.cpp'))
SRCDIRS := $(sort $(dir $(shell find . -follow -name '*.cpp')))
OBJECTS := $(patsubst %.cpp,%.o,$(SOURCES))
INCLUDE += $(patsubst %,-I%,$(SRCDIRS))
VPATH   := $(SRCDIRS)

CC      := g++
BIN     := /usr/local/bin
LIB     := /usr/local/lib
CFLAGS  := $(INCLUDE) -Wall -ansi -pipe
LDFLAGS := -lm -lc -lpthread

all: CPPFLAGS += -O2 -fomit-frame-pointer
all: CFLAGS += -O2 -fomit-frame-pointer
all: $(PROJECT)

$(PROJECT): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(PROJECT) $(OBJECTS) $(LDFLAGS)
	strip $(PROJECT)

debug: CPPFLAGS += -O0 -g
debug: CFLAGS += -O0 -g
debug: $(OBJECTS)
	$(CC) $(CFLAGS) -o $(PROJECT)_dbg $(OBJECTS) $(LDFLAGS)

clean:
	rm -f core *.o *.a *.log $(PROJECT) $(PROJECT)_dbg
