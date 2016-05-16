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

TESTFILE_FAT16_LARGE := ./_test_fat16_large.bin
TESTFILE_FAT32_LARGE := ./_test_fat32_large.bin

TESTFS       := testfs
TESTFS_FILES := $(shell find $(TESTFS) -mindepth 1 -print)

TESTFS_LARGE       := testfs_large
TESTFS_LARGE_FILES := $(shell find $(TESTFS_LARGE) -mindepth 1 -print)

TESTFILES := \
	$(TESTFILE_FAT12) \
	$(TESTFILE_FAT16) \
	$(TESTFILE_FAT32) \
	$(TESTFILE_FAT16_LARGE) \
	$(TESTFILE_FAT32_LARGE)

CXXFLAGS += \
	-DMUTEST_FAT12FILE=\"$(TESTFILE_FAT12)\" \
	-DMUTEST_FAT16FILE=\"$(TESTFILE_FAT16)\" \
	-DMUTEST_FAT32FILE=\"$(TESTFILE_FAT32)\" \
	-DMUTEST_FAT16FILE_LARGE=\"$(TESTFILE_FAT16_LARGE)\" \
	-DMUTEST_FAT32FILE_LARGE=\"$(TESTFILE_FAT32_LARGE)\"

.PHONY: test clean

test: $(BINFILES) $(TESTFILES)
	@for f in $(BINFILES); \
	do \
	echo "\nTesting $$f\n------------------------------"; \
	"./$$f"; \
	done

clean:
	rm -vf  $(TESTFILES)
	rm -vf  tests.log
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

$(TESTFS_LARGE)/rtdir100/huge.txt: $(TESTFS_LARGE)/genfiles.pl
	$<

$(TESTFILE_FAT16_LARGE): $(TESTFS_FILES_LARGE) $(TESTFS_LARGE)/rtdir100/huge.txt
	head -c $$((1024 * 1024 * 32)) /dev/zero > $@
	mkfs.vfat -n MUSTORETEST -F16 -f1 $@
	mcopy -s $(TESTFS_LARGE)/* ::/ -i $@

$(TESTFILE_FAT32_LARGE): $(TESTFS_FILES_LARGE) $(TESTFS_LARGE)/rtdir100/huge.txt
	head -c $$((1024 * 1024 * 64)) /dev/zero > $@
	mkfs.vfat -n MUSTORETEST -F32 -f1 $@
	mcopy -s $(TESTFS_LARGE)/* ::/ -i $@
