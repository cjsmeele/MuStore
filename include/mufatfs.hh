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
private:
    struct NodeContext {
        size_t lbaStart;
        size_t lbaCurrent;
    };
    static_assert(sizeof(NodeContext) <= MuFsNode::CONTEXT_SIZE,
                  "FS context size exceeds reserved space in MuFsNode type"
                  " (please increase MuFsNode::CONTEXT_SIZE)"
                 );

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
    uint16_t logicalSectorSize = 0; ///< Must be 512 (we do not support other values).
    uint8_t  clusterSize       = 0; ///< In blocks.
    uint16_t reservedBlocks    = 0;
    uint32_t blockCount        = 0;
    uint32_t fatSize           = 0;
    // }}}

    SubType subType = SubType::NONE;

    size_t  fatCacheLba = 0; ///< LBA of the currently cached FAT block. There's never a FAT at LBA 0.
    uint8_t fatCache[MAX_BLOCK_SIZE];   ///< Hold one FAT block in memory.

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
