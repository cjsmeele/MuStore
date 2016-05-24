/**
 * \file
 * \brief     MuScaleBlockStore header.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#pragma once

#include "mublockstore.hh"

/**
 * \brief MuBlockStore decorator for upscaling block sizes.
 *
 * This allows one to access stores with smaller block sizes as if
 * they have a larger block size. A single read or write call results
 * in multiple operations of that type on the underlying store.
 */
class MuScaleBlockStore : public MuBlockStore {

private:
    /// The store we pass calls to.
    MuBlockStore *store;

    /// Block size scale. The block size of this store must be a
    /// multiple of that of the underlying block store.
    size_t scale;

public:
    MuBlockStoreError seek(size_t lba);

    MuBlockStoreError read (void *buffer);
    MuBlockStoreError write(const void *buffer);

    using MuBlockStore::read;
    using MuBlockStore::write;

    MuScaleBlockStore(MuBlockStore *store_, size_t blockSize);

    ~MuScaleBlockStore() = default;
};
