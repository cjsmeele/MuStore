/**
 * \file
 * \brief     FatFs header.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 *
 * \page License
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include "fs.hh"

namespace MuStore {

/**
 * \brief FAT filesystem.
 */
class FatFs : public Fs {

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

    StoreError readBlock(size_t lba, void *buffer);
    StoreError writeBlock(size_t lba, const void *buffer);

    StoreError readCacheBlock(size_t lba, void *cache, size_t &cacheLba);
    StoreError writeCacheBlock(size_t lba, const void *buffer, void *cache, size_t &cacheLba);

    StoreError readFatBlock (size_t blockNo, void **buffer);
    StoreError readRootBlock(size_t blockNo, void **buffer);
    StoreError readDataBlock(size_t blockNo, void **buffer);

    StoreError writeFatBlock (size_t blockNo, const void *buffer);
    StoreError writeRootBlock(size_t blockNo, const void *buffer);
    StoreError writeDataBlock(size_t blockNo, const void *buffer);

    /// Magic end-of-chain lba, used internally.
    static const size_t BLOCK_EOC    = ~(size_t)0ULL;
    /// FAT cluster EOC marker. NOTE: this is not the only possible EOC marker value!
    static const size_t CLUSTER_EOC = ~(size_t)0ULL;
    /// FAT cluster free marker.
    static const size_t CLUSTER_FREE = (size_t)0ULL;

    inline size_t blockToCluster(size_t blockNo) const {
        // Add the two reserved clusters.
        if (blockNo == BLOCK_EOC)
            return CLUSTER_EOC;
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

    FsError getFatEntry(size_t clusterNo, size_t &entry);
    FsError setFatEntry(size_t clusterNo, size_t nextCluster);

    FsError allocCluster(size_t currentCluster, size_t &nextCluster);

    FsError  readNodeBlock(FsNode &node, void **buffer);
    FsError writeNodeBlock(FsNode &node, const void *buffer);
    FsError   incNodeBlock(FsNode &node, bool allocate = false);

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

    FsNode getRoot(FsError &err);
    FsNode readDir(FsNode &parent, FsError &err);

    FsError removeNode(FsNode &node);
    FsError renameNode(FsNode &node, const char *newName);
    FsError   moveNode(FsNode &node, const char *newPath);
    FsNode mkdir (FsNode &parent, const char *name, FsError &err);
    FsNode mkfile(FsNode &parent, const char *name, FsError &err);

    FsError seek (FsNode &node, size_t pos_);
    size_t read (FsNode &file,       void *buffer, size_t size, FsError &err);
    size_t write(FsNode &file, const void *buffer, size_t size, FsError &err);

    FsError truncate(FsNode &file);

    /**
     * \brief FatFs constructor.
     *
     * \param store_ the storage backend to use
     */
    FatFs(Store *store_);
    ~FatFs() = default;
};

}
