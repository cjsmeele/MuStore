/**
 * \file
 * \brief     MuFileBlockStore header.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#pragma once

#include "mublockstore.hh"
#include <cstdio>

/**
 * \brief MuBlockStore file backend.
 *
 * This uses a single file as a block storage backend.
 */
class MuFileBlockStore : public MuBlockStore {

private:
    /// The storage backend is a stdio file handle.
    FILE *fh;

    /**
     * \brief Close the file, disallow any further I/O operations.
     *
     * Use this for unrecoverable errors.
     */
    void close();

public:
    MuBlockStoreError seek(size_t lba);

    MuBlockStoreError read (void *buffer);
    MuBlockStoreError write(const void *buffer);

    // Needed because we overload read and write methods.
    using MuBlockStore::read;
    using MuBlockStore::write;

    /**
     * \param path path to the file that will be used as a storage backend
     * \param writable whether to allow write access to the file
     */
    MuFileBlockStore(const char *path, bool writable = true);
    ~MuFileBlockStore();
};
