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

    /// Magic end-of-chain block value.
    static const size_t BLOCK_EOC = ~(size_t)0ULL;

    inline size_t blockToCluster(size_t blockNo) const {
        // Add the two reserved clusters.
        return blockNo / clusterSize + 2;
    }
    inline size_t clusterToBlock(size_t clusterNo) const {
        if (clusterNo < 2
            || clusterNo > 0x0fffffef
            || (subType == SubType::FAT16 && clusterNo > 0xffef)
            || (subType == SubType::FAT12 && clusterNo > 0x0fef))
            return BLOCK_EOC;
        else
            return (clusterNo - 2) * clusterSize;
    }

    MuFsError getNodeBlock(MuFsNode &node, void **buffer);
    MuFsError incNodeBlock(MuFsNode &node);

public:
    const char *getFsType() const { return "FAT";   }

    /**
     * \brief Get the 'subtype' of this FAT filesystem
     *
     * \retval SubType::NONE for unrecognized filesystems
     * \retval SubType::FAT12
     * \retval SubType::FAT16
     * \retval SubType::FAT32
     */
    SubType  getFsSubType() const { return subType; }

    /**
     * \brief Check whether this FS is case sensitive.
     *
     * FAT is case insensitive.
     *
     * \return false
     */
    bool isCaseSensitive()  const { return false;   }

    MuFsNode getRoot(MuFsError &err);
    MuFsNode readDir(MuFsNode &parent, MuFsError &err);

    MuFsError seek (MuFsNode &node, size_t pos_);
    size_t read (MuFsNode &file,       void *buffer, size_t size, MuFsError &err);
    size_t write(MuFsNode &file, const void *buffer, size_t size, MuFsError &err);

    /**
     * \brief MuFatFs constructor.
     *
     * \param store_ the storage backend to use
     */
    MuFatFs(MuBlockStore &store_);
    ~MuFatFs() = default;
};
