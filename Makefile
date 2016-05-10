# Copyright (c) 2016, Chris Smeele

SRCDIR := ./src
OBJDIR := ./obj

#CXXFILES := $(shell find $(SRCDIR) -name "*.cc" -print)
CXXFILES :=
HXXFILES := $(shell find $(SRCDIR) -name "*.hh" -print)
BINFILE  := libmustore.a

CXXWARNINGS := \
	-Wall \
	-Wextra \
	-Wshadow \
	-Wpedantic \
	-Wpointer-arith \
	-Wno-int-to-pointer-cast \
	-Wcast-align \
	-Wwrite-strings \
	-Wmissing-declarations \
	-Wredundant-decls \
	-Winline \
	-Wuninitialized \
	-Wconversion

CXXFLAGS := \
	-fdiagnostics-color=auto \
	-Os \
	-g0 \
	-std=c++11 \
	-pipe \
	-I. \
	-I./include \
	$(CXXWARNINGS)

MUSTORE_ENABLE_BLOCK ?= file
MUSTORE_ENABLE_FS    ?=
MUSTORE_ENABLE_VFS   ?= 1

-include Makefile.local

ifneq (,$(findstring file,$(MUSTORE_ENABLE_BLOCK)))
CXXFILES += $(SRCDIR)/mufileblockstore.cc
endif
ifneq (,$(findstring mem,$(MUSTORE_ENABLE_BLOCK)))
CXXFILES += $(SRCDIR)/mumemblockstore.cc
endif
ifneq (,$(findstring fat,$(MUSTORE_ENABLE_FS)))
CXXFILES += $(SRCDIR)/mufatfs.cc
endif

OBJFILES := $(CXXFILES:$(SRCDIR)/%.cc=$(OBJDIR)/%.o)

.PHONY: all doc test clean

all: $(BINFILE)

doc: $(HXXFILES) doxygen.conf

clean:
	rm  -vf $(BINFILE)
	rm -rvf $(OBJDIR)

$(BINFILE): $(OBJFILES)
	$(AR) rcs $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.cc
	mkdir -p $(OBJDIR)
	$(CXX) $(CXXFLAGS) -c -o $@ $^
