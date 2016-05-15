/**
 * \file
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#include "mufatfs.hh"

#include <cstring>
#include <cstdio>

// Directory operations {{{

MuFsNode MuFatFs::getRoot(MuFsError &err) {
    err = MUFS_ERR_OK;
    return makeNode("/", true, true);
}

MuFsNode MuFatFs::readDir(MuFsNode &parent, MuFsError &err) {
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
