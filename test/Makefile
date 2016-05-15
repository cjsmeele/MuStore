SRCDIR := ./src
BINDIR := ./bin

CXXFILES := $(shell find $(SRCDIR) -name "*.cc" -print | sort)
HXXFILES := $(shell find $(SRCDIR) -name "*.hh" -print)
BINFILES := $(CXXFILES:$(SRCDIR)/%.cc=$(BINDIR)/%)

CXXFLAGS := -Wall -Wextra -Wpedantic -O0 -g3 -std=c++11 -I. -I../include
LDFLAGS  := -L.. -lmustore

TESTFILE_FAT12 := ./_test_fat12.bin
TESTFILE_FAT16 := ./_test_fat16.bin
TESTFILE_FAT32 := ./_test_fat32.bin

TESTFS := testfs
TESTFS_FILES := $(shell find $(TESTFS) -mindepth 1 -print)

TESTFILES := \
	$(TESTFILE_FAT12) \
	$(TESTFILE_FAT16) \
	$(TESTFILE_FAT32)

CXXFLAGS += \
	-DMUTEST_FAT12FILE=\"$(TESTFILE_FAT12)\" \
	-DMUTEST_FAT16FILE=\"$(TESTFILE_FAT16)\" \
	-DMUTEST_FAT32FILE=\"$(TESTFILE_FAT32)\"

.PHONY: test clean

test: $(BINFILES) $(TESTFILES)
	@for f in $(BINFILES); \
	do \
	echo "\nTesting $$f\n------------------------------"; \
	"./$$f"; \
	done

clean:
	rm -vf  $(TESTFILES)
	rm -rvf $(BINDIR)

$(BINDIR)/%: $(SRCDIR)/%.cc ../libmustore.a $(HXXFILES)
	@mkdir -p $(BINDIR)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LDFLAGS)

$(TESTFILE_FAT12): $(TESTFS_FILES)
	head -c $$((1024 * 128)) /dev/zero > $@
	mkfs.vfat -n MUSTORETEST -F12 -f1 $@
	mcopy -s $(TESTFS)/* ::/ -i $@

$(TESTFILE_FAT16): $(TESTFS_FILES)
	head -c $$((1024 * 1024 * 32)) /dev/zero > $@
	mkfs.vfat -n MUSTORETEST -F16 -f1 $@
	mcopy -s $(TESTFS)/* ::/ -i $@

$(TESTFILE_FAT32): $(TESTFS_FILES)
	head -c $$((1024 * 1024 * 64)) /dev/zero > $@
	mkfs.vfat -n MUSTORETEST -F32 -f1 $@
	mcopy -s $(TESTFS)/* ::/ -i $@
