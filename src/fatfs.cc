/**
 * \file
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file LICENSE or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */
#include "fatfs.hh"

#include <cstring>
#include <algorithm>

namespace MuStore {

// We distinguish FAT types using the number of clusters.
// Source: http://www.win.tue.nl/~aeb/linux/fs/fat/fat-1.html
static const size_t FAT12_MAX_CLUSTER_COUNT = 4084;
static const size_t FAT16_MAX_CLUSTER_COUNT = 65524;
// If the number of clusters is greater than the maximum for FAT16, FAT32 is assumed.

struct NodeContext {
    size_t startBlock;        ///< Relative to FAT region (fatLba / rootLba / dataLba).
    size_t currentBlock;      ///< .
    size_t currentEntry;      ///< Current direntry, only used for directories.
    size_t parentLba;         ///< Directory LBA that contains the dirent for this node.
    size_t parentBlockOffset; ///< Offset directory entries to this node's entry within parentLba.
};
static_assert(sizeof(NodeContext) <= FsNode::CONTEXT_SIZE,
              "FS context size exceeds reserved space in FsNode type"
              " (please increase FsNode::CONTEXT_SIZE)"
             );

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
    struct BiosParameterBlock {
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
            uint8_t  extendedBootSignature; ///< 0x29 if the following fields are valid, 0x28 otherwise.
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
            uint32_t rootCluster;
            uint16_t fsInfoBlock;
            uint16_t fatCopyBlock; ///< 3 blocks.
            uint8_t  _reserved1[12];
            uint8_t  driveNumber;
            uint8_t  _reserved2;
            uint8_t  extendedBootSignature; ///< 0x29 if the following fields are valid, 0x28 otherwise.
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

static void trimName(char *str, size_t size) {
    if (!size)
        return;

    for (size_t i = size-1; ; i--) {
        if (str[i] == ' ')
            str[i] = '\0';
        else if (str[i])
            break;

        if (i == 0)
            break;
    }
}

StoreError FatFs::readBlock(size_t lba, void *buffer) {
    return store->read(lba, buffer);
}
StoreError FatFs::writeBlock(size_t lba, const void *buffer) {
    return store->write(lba, buffer);
}

StoreError FatFs::readCacheBlock(size_t lba, void *cache, size_t &cacheLba) {
    if (lba == cacheLba) {
        // Nothing to do :D
        return STORE_ERR_OK;
    } else {
        // Cache the requested block.
        auto err = readBlock(lba, cache);
        cacheLba = err ? 0 : lba; // Invalidate the cache on error.
        return err;
    }
}

StoreError FatFs::writeCacheBlock(size_t lba, const void *buffer, void *cache, size_t &cacheLba) {
    auto err = writeBlock(lba, buffer);
    if (err) {
        if (buffer == cache)
            cacheLba = 0; // Invalidate the cache if it was used.
        return err;
    }

    cacheLba = lba;

    // Did we write from cache?
    if (buffer == cache) {
        // Yes. Cache is already up-to-date.
    } else {
        // Cache the written block.
        memcpy(cache, buffer, logicalSectorSize);
    }

    return STORE_ERR_OK;
}

StoreError FatFs::readFatBlock(size_t blockNo, void **buffer) {
    auto err = readCacheBlock(fatLba + blockNo, fatCache, fatCacheLba);
    if (!err)
        *buffer = fatCache;
    return err;
}

StoreError FatFs::readDataBlock(size_t blockNo, void **buffer) {
    auto err = readCacheBlock(dataLba + blockNo, dataCache, dataCacheLba);
    if (!err)
        *buffer = dataCache;
    return err;
}

// Note: not valid for FAT32.
StoreError FatFs::readRootBlock(size_t blockNo, void **buffer) {
    auto err = readCacheBlock(rootLba + blockNo, dataCache, dataCacheLba);
    if (!err)
        *buffer = dataCache;
    return err;
}

StoreError FatFs::writeFatBlock(size_t blockNo, const void *buffer) {
    return writeCacheBlock(fatLba + blockNo, buffer, fatCache, fatCacheLba);
}

StoreError FatFs::writeDataBlock(size_t blockNo, const void *buffer) {
    return writeCacheBlock(dataLba + blockNo, buffer, dataCache, dataCacheLba);
}

// Note: not valid for FAT32.
StoreError FatFs::writeRootBlock(size_t blockNo, const void *buffer) {
    return writeCacheBlock(rootLba + blockNo, buffer, dataCache, dataCacheLba);
}

FsError FatFs::readNodeBlock(FsNode &node, void **buffer) {
    NodeContext *ctx = static_cast<NodeContext*>(getNodeContext(node));

    if (!strcmp(node.getName(), "/") && (subType == SubType::FAT12 || subType == SubType::FAT16)) {
        // Special handling for the root directory region in FAT1x.

        if (ctx->currentBlock >= rootDirEntryCount * sizeof(DirEntry) / logicalSectorSize)
            // Hard limit of root directory reached.
            return FS_EOF;

        auto blockErr = readRootBlock(ctx->currentBlock, buffer);
        if (blockErr)
            return FS_ERR_IO;

        return FS_ERR_OK;

    } else {
        if (ctx->currentBlock == BLOCK_EOC)
            return FS_EOF;

        auto blockErr = readDataBlock(ctx->currentBlock, buffer);
        if (blockErr)
            return FS_ERR_IO;

        return FS_ERR_OK;
    }
}

FsError FatFs::writeNodeBlock(FsNode &node, const void *buffer) {
    NodeContext *ctx = static_cast<NodeContext*>(getNodeContext(node));

    if (!strcmp(node.getName(), "/") && (subType == SubType::FAT12 || subType == SubType::FAT16)) {

        if (ctx->currentBlock >= rootDirEntryCount * sizeof(DirEntry) / logicalSectorSize)
            // Hard limit of root directory reached.
            return FS_ERR_NO_SPACE;

        auto blockErr = writeRootBlock(ctx->currentBlock, buffer);
        if (blockErr)
            return FS_ERR_IO;

        return FS_ERR_OK;

    } else {
        if (ctx->currentBlock == BLOCK_EOC)
            // A new block should have already been allocated.
            return FS_EOF;

        auto blockErr = writeDataBlock(ctx->currentBlock, buffer);
        if (blockErr)
            return FS_ERR_IO;

        return FS_ERR_OK;
    }
}

FsError FatFs::getFatEntry(size_t clusterNo, size_t &entry) {
    size_t currentCluster = clusterNo;
    size_t nextCluster    = 0;
    void  *buffer;

    if (subType == SubType::FAT12) {
        // Ugh.

        size_t byteOff = currentCluster * 3 / 2; // N 1-and-a-half bytes.

        auto err = readFatBlock(byteOff / logicalSectorSize, &buffer);
        if (err)
            return FS_ERR_IO;

        nextCluster |=
            currentCluster & 1
            ? ((uint8_t*)buffer)[byteOff % 512] >> 4
            : ((uint8_t*)buffer)[byteOff % 512];

        byteOff++;

        // It seems FAT12 cluster numbers can be spread over multiple FAT sectors.
        // I do not like this.
        err = readFatBlock(byteOff / logicalSectorSize, &buffer);
        if (err)
            return FS_ERR_IO;

        nextCluster |=
            currentCluster & 1
            ? (uint32_t)((uint8_t*)buffer)[byteOff % 512] << 4
            : ((uint32_t)((uint8_t*)buffer)[byteOff % 512] & 0x0f) << 8;

    } else {
        // Yay.
        uint32_t clustersPerFatSector =
            subType == SubType::FAT16
            ? logicalSectorSize / 2
            : logicalSectorSize / 4;

        auto err = readFatBlock(currentCluster / clustersPerFatSector, &buffer);
        if (err)
            return FS_ERR_IO;

        if (subType == SubType::FAT16) {
            nextCluster = ((uint16_t*)buffer)[currentCluster % clustersPerFatSector];
        } else if (subType == SubType::FAT32) {
            nextCluster = ((uint32_t*)buffer)[currentCluster % clustersPerFatSector];
        }
    }

    entry = nextCluster;

    return FS_ERR_OK;
}

FsError FatFs::setFatEntry(size_t clusterNo, size_t nextCluster) {
    void *buffer;

    if (subType == SubType::FAT12) {
        // Ugh.

        size_t byteOff = clusterNo * 3 / 2; // N 1-and-a-half bytes.

        auto err = readFatBlock(byteOff / logicalSectorSize, &buffer);
        if (err)
            return FS_ERR_IO;

        ((uint8_t*)buffer)[byteOff % 512] =
            clusterNo & 1
            ? (uint8_t)((((uint8_t*)buffer)[byteOff % 512] & 0x0f)
               | (uint8_t)((nextCluster & 0x0f)<<4))
            : (uint8_t)nextCluster;

        err = writeFatBlock(byteOff / logicalSectorSize, buffer);
        if (err)
            return FS_ERR_IO;

        byteOff++;

        if (byteOff % logicalSectorSize == 0) {
            // We crossed a sector boundary.
            err = readFatBlock(byteOff / logicalSectorSize, &buffer);
            if (err)
                return FS_ERR_IO;
        }

        ((uint8_t*)buffer)[byteOff % 512] =
            clusterNo & 1
            ? (uint8_t)(nextCluster >> 4)
            : (uint8_t)((((uint8_t*)buffer)[byteOff % 512] & 0xf0)
               | (uint8_t)(nextCluster >> 8));

        err = writeFatBlock(byteOff / logicalSectorSize, buffer);
        if (err)
            return FS_ERR_IO;

    } else {
        // Yay.
        uint32_t clustersPerFatSector =
            subType == SubType::FAT16
            ? logicalSectorSize / 2
            : logicalSectorSize / 4;

        auto err = readFatBlock(clusterNo / clustersPerFatSector, &buffer);
        if (err)
            return FS_ERR_IO;

        if (subType == SubType::FAT16) {
            ((uint16_t*)buffer)[clusterNo % clustersPerFatSector] = (uint16_t)nextCluster;
        } else if (subType == SubType::FAT32) {
            ((uint32_t*)buffer)[clusterNo % clustersPerFatSector] = (uint32_t)nextCluster;
        }

        err = writeFatBlock(clusterNo / clustersPerFatSector, buffer);
        if (err)
            return FS_ERR_IO;
    }

    return FS_ERR_OK;
}

FsError FatFs::allocCluster(size_t currentCluster, size_t &nextCluster) {
    // Find the first empty cluster.
    nextCluster = 0;

    // NOTE: This is not very efficient, especially because FAT32 has
    // a FS Information Sector that can provide useful info wrt
    // finding the next free cluster. Even adding our own in-memory
    // bookkeeping would speed this up quite a bit.
    for (size_t i = 2; i < dataClusterCount; i++) {
        size_t entry;
        auto err = getFatEntry(i, entry);
        if (err)
            return err;
        if (entry == CLUSTER_FREE) {
            nextCluster = i;
            break;
        }
    }

    if (!nextCluster)
        // No free clusters available.
        return FS_ERR_NO_SPACE;

    if (currentCluster) {
        // Update the current FAT entry to point to the new cluster.
        auto err = setFatEntry(currentCluster, nextCluster);
        if (err)
            return err;

        err = setFatEntry(nextCluster, CLUSTER_EOC);
        if (err)
            return err;
    }

    return FS_ERR_OK;
}

FsError FatFs::incNodeBlock(FsNode &node, bool allocate) {
    NodeContext *ctx = static_cast<NodeContext*>(getNodeContext(node));

    if (!strcmp(node.getName(), "/") && (subType == SubType::FAT12 || subType == SubType::FAT16)) {
        // FIXME: Return EOF when going past the FAT12/FAT16 directory
        // section (reads and writes will already fail).
        ctx->currentBlock++; // No need for cluster magic! :D
        return FS_ERR_OK;

    } else {
        if ((ctx->currentBlock+1) % clusterSize) {
            // We'll remain in the same cluster, no problemo.
            ctx->currentBlock++;
            return FS_ERR_OK;
        } else {
            // We need to find the next cluster.

            if (ctx->currentBlock == BLOCK_EOC) {
                // We already encountered the EOC, nothing to do.
                return FS_EOF;
            }
            size_t nextCluster = 0;

            auto err = getFatEntry(blockToCluster(ctx->currentBlock), nextCluster);
            if (err)
                return err;

            if (clusterToBlock(nextCluster) == BLOCK_EOC && allocate) {
                // We are at EOC, allocate a new cluster.
                err = allocCluster(blockToCluster(ctx->currentBlock), nextCluster);
                if (err)
                    return err;
            }

            ctx->currentBlock = clusterToBlock(nextCluster);

            return FS_ERR_OK;
        }
    }
}

// Directory operations {{{

FsNode FatFs::getRoot(FsError &err) {
    if (subType == SubType::NONE) {
        err = FS_ERR_OPER_UNAVAILABLE;
        return {this};
    }

    err = FS_ERR_OK;
    auto node = makeNode("/", true, true);
    NodeContext *ctx = static_cast<NodeContext*>(getNodeContext(node));

    if (subType == SubType::FAT12 || subType == SubType::FAT16)
        ctx->startBlock = 0;
    else
        ctx->startBlock = clusterToBlock(rootCluster);

    ctx->currentBlock      = ctx->startBlock;
    ctx->currentEntry      = 0;
    ctx->parentLba         = 0;
    ctx->parentBlockOffset = 0;

    return node;
}

FsNode FatFs::readDir(FsNode &parent, FsError &err) {
    if (subType == SubType::NONE) {
        err = FS_ERR_OPER_UNAVAILABLE;
        return {this};
    }

    NodeContext *ctx = static_cast<NodeContext*>(getNodeContext(parent));

    if (!parent.doesExist()) {
        err = FS_ERR_OBJECT_NOT_FOUND;
        return {this};
    } else if (!parent.isDirectory()) {
        err = FS_ERR_NOT_DIRECTORY;
        return {this};
    }

    void    *buffer = nullptr;
    DirEntry *entry = nullptr;

    // Fetch the next regular direntry, skip 'disk' and 'volume label' types.
    bool gotEntry = false;
    do {
        err = readNodeBlock(parent, &buffer);
        if (err)
            return {this};
        if ((ctx->currentEntry + 1) % (logicalSectorSize / sizeof(DirEntry)) == 0) {
            err = incNodeBlock(parent);
            if (err)
                return {this};
        }

        // Adjust pointer to current directory entry.
        entry = static_cast<DirEntry*>(buffer)
              + ctx->currentEntry % (logicalSectorSize / sizeof(DirEntry));

        if (!entry->name[0]) {
            // A name field starting with a NUL byte indicates directory EOF.
            err = FS_EOF;
            return {this};
        }

        if (
            !(entry->attrDisk | entry->attrVolumeLabel)
            && (uint8_t)entry->name[0] != 0xe5 // Indicates deleted file.
        ) {
            gotEntry = true;
        }

        ctx->currentEntry++;

    } while (!gotEntry);

    nodeUpdatePos(parent, parent.getPos()+1);

    // Copy the node name.
    char name[13] = { };
    strncpy(name, entry->name, 8);
    trimName(name, 8);
    if (entry->extension[0] && entry->extension[0] != ' ')
        strcat(name, ".");
    strncat(name, entry->extension, 3);
    trimName(name, 13);

    // Create and fill the child node.
    auto child = makeNode(name, true, entry->attrDirectory,
                          entry->attrDirectory ? 0 : entry->fileSize);
    NodeContext *childCtx = static_cast<NodeContext*>(getNodeContext(child));

    uint32_t startCluster = ((uint32_t)entry->clusterNoHigh << 16) | entry->clusterNoLow;

    childCtx->startBlock        = clusterToBlock(startCluster);
    childCtx->currentBlock      = childCtx->startBlock;
    childCtx->parentBlockOffset = (ctx->currentEntry-1) % (logicalSectorSize / sizeof(DirEntry));
    if ((subType == SubType::FAT12 || subType == SubType::FAT16)
        && strcmp(parent.getName(), "/") == 0) {
        childCtx->parentLba = rootLba + ctx->currentBlock;
    } else {
        childCtx->parentLba = dataLba + ctx->currentBlock;
    }

    err = FS_ERR_OK;
    return child;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

FsError FatFs::removeNode(FsNode &node){
    // TODO.
    return FS_ERR_OPER_UNAVAILABLE;
}
FsError FatFs::renameNode(FsNode &node, const char *newName){
    // TODO.
    return FS_ERR_OPER_UNAVAILABLE;
}
FsError FatFs::moveNode(FsNode &node, const char *newPath){
    // TODO.
    return FS_ERR_OPER_UNAVAILABLE;
}
FsNode FatFs::mkdir(FsNode &parent, const char *name, FsError &err){
    // TODO.
    err = FS_ERR_OPER_UNAVAILABLE;
    return {this};
}
FsNode FatFs::mkfile(FsNode &parent, const char *name, FsError &err){
    // TODO.
    err = FS_ERR_OPER_UNAVAILABLE;
    return {this};
}

#pragma GCC diagnostic pop

FsError FatFs::truncate(FsNode &file) {
    if (!file.doesExist())
        return FS_ERR_OBJECT_NOT_FOUND;
    if (file.isDirectory())
        return FS_ERR_NOT_FILE; // Can't truncate directories (yet).

    NodeContext *ctx = static_cast<NodeContext*>(getNodeContext(file));

    size_t newSize = file.getPos();

    // Update file size first {{{
    void *buffer = dataCache;
    // Don't bother checking whether we need readDataBlock or
    // readRootBlock, this has the same effect when we already know the lba.
    auto berr = readCacheBlock(ctx->parentLba, dataCache, dataCacheLba);
    if (berr)
        return FS_ERR_IO;

    DirEntry *dentry = (DirEntry*)buffer + ctx->parentBlockOffset;
    dentry->fileSize = (uint32_t)newSize;

    berr = writeCacheBlock(ctx->parentLba, buffer, dataCache, dataCacheLba);
    if (berr)
        return FS_ERR_IO;
    // }}}

    if (ctx->currentBlock == BLOCK_EOC)
        // Already done.
        return FS_ERR_OK;

    // Round down to cluster start.
    ctx->currentBlock -= ctx->currentBlock % clusterSize;

    // Truncate the cluster chain.
    for (size_t i = 0; ; i++) {
        size_t currentBlock   = ctx->currentBlock;
        size_t currentCluster = blockToCluster(currentBlock);

        FsError err;

        if (currentBlock != BLOCK_EOC) {
            // Increment current cluster number before we overwrite it.
            for (size_t j = 0; j < clusterSize; j++) {
                err = incNodeBlock(file);
                if (err)
                    return FS_ERR_IO; // XXX: Not really the right error number.
            }
        }

        if (i == 0) {
            // Current entry should be set to EOC.
            err = setFatEntry(currentCluster, CLUSTER_EOC);
            if (err)
                return err;
        } else {
            // Mark all other entries as free.
            err = setFatEntry(currentCluster, CLUSTER_FREE);
            if (err)
                return err;
        }

        if (ctx->currentBlock == BLOCK_EOC) {
            // This was the last entry.
            nodeUpdateSize(file, newSize);
            file.rewind();
            file.seek(newSize);
            return FS_ERR_OK;
        }
    }
}

// }}}

// File I/O {{{
FsError FatFs::seek(FsNode &node, size_t pos_) {
    if (subType == SubType::NONE)
        return FS_ERR_OPER_UNAVAILABLE;

    if (!node.doesExist())
        return FS_ERR_OBJECT_NOT_FOUND;

    NodeContext *ctx = static_cast<NodeContext*>(getNodeContext(node));

    if (pos_ == 0) {
        ctx->currentBlock = ctx->startBlock;
        ctx->currentEntry = 0;
        nodeUpdatePos(node, pos_);
        return FS_ERR_OK;

    } else {
        if (node.isDirectory())
            // Directories can currently only be rewound, not sought arbitrarily.
            return FS_ERR_OPER_UNAVAILABLE;

        if (pos_ < node.getPos()) {
            // Cluster chains are a singly linked list, as such we
            // cannot seek backwards without going back to the start.
            auto err = seek(node, 0);
            if (err)
                return err;
        }
        while (node.getPos() < pos_) {
            size_t curPos = node.getPos();
            if (curPos / logicalSectorSize == pos_ / logicalSectorSize) {
                // We have reached the destination sector.
                nodeUpdatePos(node, pos_);
                return FS_ERR_OK;
            } else {
                auto err = incNodeBlock(node, true);
                if (err)
                    return err;

                // Just in case we get an error.
                nodeUpdatePos(node, curPos + logicalSectorSize);
            }
        }

        return FS_ERR_OK;
    }
}

size_t FatFs::read(FsNode &file, void *dest, size_t size, FsError &err) {
    if (subType == SubType::NONE) {
        err = FS_ERR_OPER_UNAVAILABLE;
        return 0;
    }

    if (!file.doesExist()) {
        err = FS_ERR_OBJECT_NOT_FOUND;
        return 0;
    } else if (file.isDirectory()) {
        err = FS_ERR_NOT_FILE;
        return 0;
    }

    size_t bytesRead = 0;
    void *buffer;

    if (file.getPos() >= file.getSize()) {
        err = FS_EOF;
        return 0;
    }

    // Read block-by-block until we fill the buffer or reach EOF.
    while (bytesRead < size) {
        err = readNodeBlock(file, &buffer);
        if (err)
            return bytesRead;

        size_t sectorOffset = file.getPos() % logicalSectorSize;
        size_t toCopy = std::min({
            (size_t)(size - bytesRead),               // Requested read size.
            (size_t)(file.getSize() - file.getPos()), // Remaining file size.
            (size_t)logicalSectorSize - sectorOffset  // Remaining bytes in current sector.
        });

        memcpy(
            (uint8_t*)dest   + bytesRead,
            (uint8_t*)buffer + sectorOffset,
            toCopy
        );

        bytesRead += toCopy;

        if ((file.getPos() % logicalSectorSize + toCopy) == logicalSectorSize) {
            // XXX: If EOF is reached at the end of the last sector, a
            //      new block is allocated after the last block. This
            //      makes append-writes easier, but can be slightly
            //      less space-efficient.
            err = incNodeBlock(file, true);
            if (err)
                return bytesRead;
        }

        nodeUpdatePos(file, file.getPos() + toCopy);

        if (file.getPos() >= file.getSize() && size - bytesRead > 0) {
            // EOF reached before the requested amount of bytes could be read.
            err = FS_EOF;
            return bytesRead;
        }
    }

    err = FS_ERR_OK;
    return bytesRead;
}
size_t FatFs::write(FsNode &file, const void *bufferW, size_t size, FsError &err) {
    if (subType == SubType::NONE) {
        err = FS_ERR_OPER_UNAVAILABLE;
        return 0;
    }
    if (!file.doesExist()) {
        err = FS_ERR_OBJECT_NOT_FOUND;
        return 0;
    } else if (file.isDirectory()) {
        err = FS_ERR_NOT_FILE;
        return 0;
    }

    NodeContext *ctx = static_cast<NodeContext*>(getNodeContext(file));

    size_t origPos = file.getPos();
    size_t bytesWritten = 0;
    void *bufferR;

    // Update the dirent for this node to match the new file size.
    auto syncFileSize = [&]() {
        size_t newSize = std::max(
            origPos + bytesWritten,
            file.getSize()
        );

        void *buffer = dataCache;
        auto berr = readCacheBlock(ctx->parentLba, dataCache, dataCacheLba);
        if (berr)
            return FS_ERR_IO;

        DirEntry *dentry = (DirEntry*)buffer + ctx->parentBlockOffset;
        dentry->fileSize = (uint32_t)newSize;

        berr = writeCacheBlock(ctx->parentLba, buffer, dataCache, dataCacheLba);
        if (berr)
            return FS_ERR_IO;

        nodeUpdateSize(file, newSize);
        return FS_ERR_OK;
    };

    // Write the input buffer block-by-block.
    while (bytesWritten < size) {
        // Read current block.
        err = readNodeBlock(file, &bufferR);
        if (err) {
            syncFileSize();
            return bytesWritten;
        }

        size_t sectorOffset = file.getPos() % logicalSectorSize;
        size_t toCopy = std::min({
            (size_t)(size - bytesWritten),            // Requested write size.
            (size_t)logicalSectorSize - sectorOffset  // Remaining bytes in current sector.
        });

        memcpy(
            (uint8_t*)bufferR + sectorOffset,
            (uint8_t*)bufferW + bytesWritten,
            toCopy
        );

        // Write back.
        err = writeNodeBlock(file, bufferR);
        if (err) {
            syncFileSize();
            return bytesWritten;
        }

        bytesWritten += toCopy;

        // Go to next block on block boundaries.
        if ((file.getPos() % logicalSectorSize + toCopy) == logicalSectorSize) {
            err = incNodeBlock(file, true);
            if (err) {
                syncFileSize();
                return bytesWritten;
            }
        }

        nodeUpdatePos(file, file.getPos() + toCopy);
    }

    syncFileSize();

    err = FS_ERR_OK;
    return bytesWritten;
}
// }}}

FatFs::FatFs(Store *store_)
    : Fs(store_) {

    uint8_t buffer[MAX_BLOCK_SIZE];

    if (store->getBlockSize() < 512 || store->getBlockSize() > MAX_BLOCK_SIZE)
        return;

    auto err = store->read(0, buffer);

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
            || logicalSectorSize != store->getBlockSize())
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
        rootLba = reservedBlocks
                + fatSize * fatCount;
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
        rootCluster = br->ebpb.fat32.rootCluster;

    // Copy volume label if available.
    if ((subType == SubType::FAT12 || subType == SubType::FAT16)
        && br->ebpb.fat1x.extendedBootSignature == 0x29) {
        strncpy(volumeLabel, br->ebpb.fat1x.volumeLabel, 11);
        trimName(volumeLabel, 11);
    } else if (subType == SubType::FAT32 && br->ebpb.fat32.extendedBootSignature == 0x29) {
        strncpy(volumeLabel, br->ebpb.fat32.volumeLabel, 11);
        trimName(volumeLabel, 11);
    }

    return;

_constructFail:
    // Leave the FatFs in a recognizably unusable state.
    subType = SubType::NONE;
}

}
