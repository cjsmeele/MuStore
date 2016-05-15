/**
 * \file
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#include "mufatfs.hh"

#include <cstring>
#include <cstdio>

// FAT data structures {{{

/**
 * \brief Layout of the first sector in a FAT volume.
 */
struct BootRecord {
    /**
     * \brief FAT Bios Parameter Block.
     */
    struct {
        uint8_t  _jump[3];
        char     oemName[8];
        uint16_t blockSize;   ///< In bytes.
        uint8_t  clusterSize; ///< In blocks.
        uint16_t reservedBlocks;
        uint8_t  fatCount;
        uint16_t rootDirEntries;
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

	uint8_t _signature[2]; // 0x55 0xaa.

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

    uint8_t buffer[4096];
    if (store.getBlockSize() >= 512 && store.getBlockSize() <= 4096) {
        auto err = store.read(0, buffer);
        if (!err) {
            strncpy(volumeLabel, (char*)buffer+0x2b, 11);
        }
    }
}
