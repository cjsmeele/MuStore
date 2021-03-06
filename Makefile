# Copyright (c) 2016, Chris Smeele

SRCDIR := ./src
INCDIR := ./include
OBJDIR := ./obj

#CXXFILES := $(shell find $(SRCDIR) -name "*.cc" -print)
CXXFILES :=
HXXFILES := $(shell find $(SRCDIR) $(INCDIR) -name "*.hh" -print)
BINFILE  := libmustore.a

CXXWARNINGS += \
	-Wall \
	-Wextra \
	-Wpedantic \
	-Wshadow \
	-Wcast-align \
	-Wmissing-declarations \
	-Wredundant-decls \
	-Wuninitialized \
	-Wconversion

CXXFLAGS += \
	-std=c++11 \
	-pipe \
	-I. \
	-I./include \
	$(CXXWARNINGS)

ifdef DEBUG
CXXFLAGS += \
	-O1 \
	-g3
else
CXXFLAGS += \
	-Os \
	-g0
endif

MUSTORE_ENABLE_BLOCK ?= file mem
MUSTORE_ENABLE_FS    ?= fat

-include Makefile.local

ifneq (,$(findstring file,$(MUSTORE_ENABLE_BLOCK)))
CXXFILES += $(SRCDIR)/filestore.cc
endif
ifneq (,$(findstring mem,$(MUSTORE_ENABLE_BLOCK)))
CXXFILES += $(SRCDIR)/memstore.cc
endif

ifneq (,$(MUSTORE_ENABLE_BLOCK))
CXXFILES += $(SRCDIR)/scalestore.cc
endif

ifneq (,$(findstring fat,$(MUSTORE_ENABLE_FS)))
CXXFILES += $(SRCDIR)/fatfs.cc
endif

ifneq (,$(MUSTORE_ENABLE_FS))
CXXFILES += $(SRCDIR)/fs.cc $(SRCDIR)/fsnode.cc
endif

OBJFILES := $(CXXFILES:$(SRCDIR)/%.cc=$(OBJDIR)/%.o)

.PHONY: all doc test clean clean-all

all: $(BINFILE)

doc: $(HXXFILES) doxygen.conf
	doxygen doxygen.conf

test: $(BINFILE)
	$(MAKE) -C test

clean:
	rm  -vf $(BINFILE)
	rm -rvf $(OBJDIR)

clean-all: clean
	$(MAKE) -C test clean

$(BINFILE): $(OBJFILES)
	$(AR) rcs $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cc $(HXXFILES)
	@mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
