/**
 * \file
 * \brief     MuFatFs header.
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#pragma once

#include "mufs.hh"

/**
 * \brief FAT filesystem.
 */
class MuFatFs : public MuFs {

public:
    enum class SubType {
        NONE = 0,
        FAT12,
        FAT16,
        FAT32,
    };

    static const size_t MAX_BLOCK_SIZE = 512;

private:
    // (E)BPB information. {{{
    uint16_t logicalSectorSize = 0; ///< Must be 512 and equal to the block size (we do not currently support other values).
    uint8_t  fatCount          = 0;
    uint32_t fatSize           = 0; ///< In blocks.
    uint8_t  clusterSize       = 0; ///< In blocks.
    uint8_t  clusterCount      = 0;
    uint16_t reservedBlocks    = 0;

    size_t fatLba            = 0;
    size_t rootLba           = 0; ///< FAT1x only.
    size_t dataLba           = 0;

    size_t rootDirEntryCount = 0;
    size_t rootCluster       = 0; ///< FAT32 only.

    size_t blockCount       = 0;
    size_t dataBlockCount   = 0;
    size_t dataClusterCount = 0;
    // }}}

    SubType subType = SubType::NONE;

    size_t  fatCacheLba = 0; ///< LBA of the currently cached FAT block.
    uint8_t fatCache[MAX_BLOCK_SIZE];

    size_t  dataCacheLba = 0; ///< LBA of the currently cached data block.
    uint8_t dataCache[MAX_BLOCK_SIZE];

    MuBlockStoreError getBlock(size_t blockNo, void *buffer);
    MuBlockStoreError getCacheBlock(size_t lba, void *cache, size_t &cacheLba);

    MuBlockStoreError getFatBlock (size_t blockNo, void **buffer);
    MuBlockStoreError getRootBlock(size_t blockNo, void **buffer);
    MuBlockStoreError getDataBlock(size_t blockNo, void **buffer);

    MuFsError getNodeBlock(MuFsNode &node, void **buffer);
    MuFsError incNodeBlock(MuFsNode &node);

public:
    const char *getFsType() const { return "FAT"; }
    SubType  getFsSubType() const { return subType; }

    // Directory operations {{{
    MuFsNode getRoot(MuFsError &err);
    MuFsNode readDir(MuFsNode &parent, MuFsError &err);
    // }}}

    // File I/O {{{
    MuFsError seek (MuFsNode &file, size_t pos_);
    MuFsError read (MuFsNode &file, void *buffer, size_t size);
    MuFsError write(MuFsNode &file, const void *buffer, size_t size);
    // }}}

    MuFatFs(MuBlockStore &store_);
    ~MuFatFs() = default;
};
