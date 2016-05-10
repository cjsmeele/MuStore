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
 */
class MuFileBlockStore : public MuBlockStore {

private:
    FILE *fh;

    /**
     * \brief Close the file, disallow any further I/O operations.
     *
     * Use this for unrecoverable errors.
     */
    void close();

public:
    MuBlockStoreError seek(size_t blockN);

    MuBlockStoreError read (void *buffer);
    MuBlockStoreError write(const void *buffer);

    MuFileBlockStore(const char *path);
    ~MuFileBlockStore();
};

