/**
 * \file
 * \brief     MuMemBlockStore header.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#pragma once

#include "mublockstore.hh"

/**
 * \brief MuBlockStore memory backend
 */
class MuMemBlockStore : public MuBlockStore {

private:
    uint8_t *store;

public:
    MuBlockStoreError seek(size_t blockN);

    MuBlockStoreError read (void *buffer);
    MuBlockStoreError write(const void *buffer);

    using MuBlockStore::read;
    using MuBlockStore::write;

    MuMemBlockStore(void *store, size_t size);
    ~MuMemBlockStore() = default;
};

