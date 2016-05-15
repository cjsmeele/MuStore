/**
 * \file
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#include "mufatfs.hh"

#include <cstring>
#include <cstdio>

// We distinguish FAT types using the number of clusers.
// Source: http://www.win.tue.nl/~aeb/linux/fs/fat/fat-1.html
static const size_t FAT12_MAX_CLUSTER_COUNT = 4084;
static const size_t FAT16_MAX_CLUSTER_COUNT = 65524;
// If the number of clusters is greater than the maximum for FAT16, FAT32 is assumed.

// FAT data structures {{{

/**
 * \brief Layout of the first sector in a FAT volume.
 */
struct BootRecord {

    uint8_t  _jump[3];
    char     oemName[8];

    /**
     * \brief FAT Bios Parameter Block.
     */
    struct {
        uint16_t blockSize;   ///< In bytes.
        uint8_t  clusterSize; ///< In blocks.
        uint16_t reservedBlocks;
        uint8_t  fatCount;
        uint16_t rootDirEntryCount;
        uint16_t blockCountShort; // Use the 32-bit blockCount field if zero.
        uint8_t  mediaDescriptor;
        uint16_t fatSizeShort;    ///< In blocks. Use the 32-bit EBPB fatSize field if zero.
        uint16_t sectorsPerTrack; // CHS stuff, do not use.
        uint16_t headCount;       // .
        uint32_t hiddenBlocks;
        uint32_t blockCount;
    } __attribute__((packed)) bpb;

    union ExtendedBpb {
        /**
         * \brief FAT12 & FAT16 Extended Bios Parameter Block.
         */
        struct {
            uint8_t  driveNumber;
            uint8_t  _reserved1;
            uint8_t  extendedBootSignature; ///< 29 if the following fields are valid, 28 otherwise.
            uint32_t volumeId;
            char     volumeLabel[11];
            char     fsType[8];

            uint8_t _bootCode[448];

        } __attribute__((packed)) fat1x;

        /**
         * \brief FAT32 EBPB.
         */
        struct {
            uint32_t fatSize; ///< In blocks.
            uint16_t flags1;
            uint16_t version;
            uint32_t rootDirCluster;
            uint16_t fsInfoBlock;
            uint16_t fatCopyBlock; ///< 3 blocks.
            uint8_t  _reserved1[12];
            uint8_t  driveNumber;
            uint8_t  _reserved2;
            uint8_t  extendedBootSignature; ///< 29 if the following fields are valid, 28 otherwise.
            uint32_t volumeId;
            char     volumeLabel[11];
            char     fsType[8];

            uint8_t _bootCode[420];

        } __attribute__((packed)) fat32;

    } __attribute__((packed)) ebpb;

	uint16_t signature; // 0xaa55.

} __attribute__((packed));

/**
 * \brief FAT directory entry.
 */
struct DirEntry {
	char name[8];      ///< Padded with spaces.
	char extension[3]; ///< Padded with spaces.

	// Attribute byte 1.
	bool attrReadOnly    : 1;
	bool attrHidden      : 1;
	bool attrSystem      : 1;
	bool attrVolumeLabel : 1;
	bool attrDirectory   : 1;
	bool attrArchive     : 1;
	bool attrDisk        : 1;
	bool _attrReserved   : 1;

	uint8_t  attributes2; ///< This byte has lots of different interpretations and we can safely ignore it.

	uint8_t  createTimeHiRes;
	uint16_t createTime;
	uint16_t createDate;
	uint16_t accessDate;
	uint16_t clusterNoHigh;
	uint16_t modifyTime;
	uint16_t modifyDate;
	uint16_t clusterNoLow;
	uint32_t fileSize; ///< In bytes.

} __attribute__((packed));

// }}}

// Directory operations {{{

MuFsNode MuFatFs::getRoot(MuFsError &err) {
    err = MUFS_ERR_OK;
    return makeNode("/", true, true);
}

MuFsNode MuFatFs::readDir(MuFsNode &parent, MuFsError &err) {
    NodeContext *ctx = static_cast<NodeContext*>(getNodeContext(parent));

    // TODO.

    err = MUFS_ERR_OPER_UNAVAILABLE;
    return {*this};
}

// }}}

// File I/O {{{
MuFsError MuFatFs::seek (MuFsNode &file, size_t pos_) {
    // TODO.
    return MUFS_ERR_OPER_UNAVAILABLE;
}
MuFsError MuFatFs::read (MuFsNode &file, void *buffer, size_t size) {
    // TODO.
    return MUFS_ERR_OPER_UNAVAILABLE;
}
MuFsError MuFatFs::write(MuFsNode &file, const void *buffer, size_t size) {
    // TODO.
    return MUFS_ERR_OPER_UNAVAILABLE;
}
// }}}

MuFatFs::MuFatFs(MuBlockStore &store_)
    : MuFs(store_) {

    uint8_t buffer[MAX_BLOCK_SIZE];

    if (store.getBlockSize() < 512 || store.getBlockSize() > MAX_BLOCK_SIZE)
        return;

    auto err = store.read(0, buffer);

    if (err)
        return;

    // Attempt to parse the information present in the boot sector.

    BootRecord *br = (BootRecord*)buffer;

    if (br->signature != 0xaa55)
        // (Yes, this is a valid use case for the goto keyword when you're not using exceptions)
        goto _constructFail;

    {
        logicalSectorSize = br->bpb.blockSize;

        if (   logicalSectorSize != MAX_BLOCK_SIZE
            || logicalSectorSize != store.getBlockSize())
            goto _constructFail;
    } { /////////////////////////////////////////////////////
        reservedBlocks = br->bpb.reservedBlocks;
        fatLba = reservedBlocks;

        if (reservedBlocks < 1)
            // At the least the boot sector should be reserved.
            goto _constructFail;
    } { /////////////////////////////////////////////////////
        clusterSize = br->bpb.clusterSize;

        if (!clusterSize)
            // We'd rather not divide by zero later on.
            goto _constructFail;
    } { /////////////////////////////////////////////////////
        blockCount = br->bpb.blockCountShort;

        if (!blockCount)
            blockCount = br->bpb.blockCount;
    } { /////////////////////////////////////////////////////
        rootDirEntryCount = br->bpb.rootDirEntryCount;
        // This field is used only for FAT12 and FAT16, as FAT32
        // stores the root directory in the data section.
        // We assume FAT32 leaves this at zero.

        if ((rootDirEntryCount * sizeof(DirEntry)) % logicalSectorSize)
            // Root direntry region size should be a divisor of sector size.
            goto _constructFail;

    } { /////////////////////////////////////////////////////
        fatSize = br->bpb.fatSizeShort;
        if (!fatSize) {
            // The BPB FAT size field is zero for FAT32. To make
            // data size calcluations work we need to know the
            // size of the FAT area, which in the case of FAT32 is
            // stored in the FAT32 EBPB. Therefore we have to
            // assume early on that the subtype is FAT32.
            fatSize = br->ebpb.fat32.fatSize;

            if (!fatSize)
                goto _constructFail;
        }
    } { /////////////////////////////////////////////////////
        fatCount = br->bpb.fatCount;

        if (!fatCount)
            goto _constructFail;
    } { /////////////////////////////////////////////////////
        dataLba = reservedBlocks
                + fatSize * fatCount
                + (rootDirEntryCount * sizeof(DirEntry) / logicalSectorSize);

        if (dataLba >= blockCount)
            goto _constructFail;
    } { /////////////////////////////////////////////////////
        dataBlockCount   = blockCount - dataLba;
        dataClusterCount = dataBlockCount / clusterSize;

        if (!dataClusterCount)
            goto _constructFail;
    }

    // Now we can reliably determine the FAT type.

    if (dataClusterCount <= FAT12_MAX_CLUSTER_COUNT)
        subType = SubType::FAT12;
    else if (dataClusterCount <= FAT16_MAX_CLUSTER_COUNT)
        subType = SubType::FAT16;
    else
        subType = SubType::FAT32;

    // A final size check.
    if (   reservedBlocks
         + fatCount * fatSize
         + rootDirEntryCount*sizeof(DirEntry) / logicalSectorSize
         + dataBlockCount
        != blockCount)
        goto _constructFail;

    // Extract information specific to FAT subtypes.

    if (subType == SubType::FAT32)
        rootDirCluster = br->ebpb.fat32.rootDirCluster;

    if (subType == SubType::FAT12 || subType == SubType::FAT16)
        strncpy(volumeLabel, br->ebpb.fat1x.volumeLabel, 11);
    else if (subType == SubType::FAT32)
        strncpy(volumeLabel, br->ebpb.fat32.volumeLabel, 11);

    return;

_constructFail:
    // Leave the MuFatFs in a recognizably unusable state.
    subType = SubType::NONE;
}
