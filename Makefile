PROJECT := BitDeque

SRCDIR := ./Source
OBJDIR := ./Objects

PROJ_SRCS := $(notdir $(shell find $(SRCDIR) -follow -name '*.cpp'))
PROJ_DIRS := $(sort $(dir $(shell find $(SRCDIR) -follow -name '*.cpp')))
PROJ_OBJS := $(patsubst %.cpp,$(OBJDIR)/%.o,$(PROJ_SRCS))
PROJ_INCL += $(patsubst %,-I%,$(PROJ_DIRS))
VPATH     := $(PROJ_DIRS)

TEST_SRCS := $(notdir $(shell find ./Test -follow -name 'Test*.cpp'))
TEST_DIRS := $(sort $(dir $(shell find ./Test -follow -name 'Test*.cpp')))
TEST_OBJS := $(patsubst %.cpp,%.o,$(TEST_SRCS))
TEST_INCL := $(patsubst %,-I%,$(TEST_DIRS))
TEST_BINS := $(patsubst %.cpp,%.utest,$(TEST_SRCS))
VPATH     += $(TEST_DIRS)

STATIC_LIB := $(PROJECT).a
SHARED_LIB := $(PROJECT).so

CC := g++
AR := ar
LD := g++
LDFLAGS := -lm -lc -lpthread
CPPFLAGS := $(PROJ_INCL) -Wall -pipe -fomit-frame-pointer -fPIC
CPPFLAGS_DEBUG := -O0 -g -fmax-errors=3
# TODO: ENABLE LOG ONLY IN DEBUG/TEST

.PHONY: all
all: CPPFLAGS += -O2
all: $(STATIC_LIB) $(SHARED_LIB)

.PHONY: debug
debug: CPPFLAGS += $(CPPFLAGS_DEBUG)
debug: $(STATIC_LIB) $(SHARED_LIB)

$(OBJDIR)/%.o: %.cpp
	$(CC) $(CPPFLAGS) -c -o $@ $<

$(STATIC_LIB): $(PROJ_OBJS)
	$(AR) rcs $(STATIC_LIB) $(PROJ_OBJS)

$(SHARED_LIB): $(PROJ_OBJS)
	$(LD) $(LDFLAGS) -shared -o $(SHARED_LIB) $(PROJ_OBJS)

.PHONY: test
test: CPPFLAGS += $(TEST_INCL) $(CPPFLAGS_DEBUG) -fprofile-arcs -ftest-coverage
test: $(TEST_BINS)
	for testbin in Test*utest; do ./$$testbin; done
	# $(COV_REPORT)

Test%.utest: Test%.o $(PROJ_OBJS)
	$(CC) $(CPPFLAGS) -o $@ $< $(PROJ_OBJS) $(LDFLAGS)

.PHONY: clean
clean:
	rm -f core *.gcno *.gcda $(OBJDIR)/*.gcno $(OBJDIR)/*.gcda \
	$(TEST_OBJS) $(TEST_BINS) $(PROJ_OBJS) $(STATIC_LIB) $(SHARED_LIB)
