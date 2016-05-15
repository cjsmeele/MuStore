/**
 * \file
 * \brief     MuFatFs header.
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#pragma once

#include "mufs.hh"

class MuFatFs : public MuFs {

public:
    enum class SubType {
        NONE = 0,
        FAT12,
        FAT16,
        FAT32,
    };

private:
    SubType subType = SubType::NONE;

public:
    const char *getFsType() const { return "FAT"; }
    SubType getFsSubType() const { return subType; }

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
