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
 * \brief MuBlockStore memory backend.
 *
 * This allows any contiguous memory region to be approached as a
 * MuBlockStore.
 */
class MuMemBlockStore : public MuBlockStore {

private:
    /// The store, represented as a memory region.
    const uint8_t *roStore;
    /// Writable store, will be `nullptr` for ro memory.
          uint8_t   *store;

public:
    MuBlockStoreError seek(size_t lba);

    MuBlockStoreError read (void *buffer);
    MuBlockStoreError write(const void *buffer);

    using MuBlockStore::read;
    using MuBlockStore::write;

    /**
     * \brief Writable blockstore constructor.
     *
     * \param store pointer to the memory region that will be used as a backend
     * \param size the size of the provided memory region
     */
    MuMemBlockStore(void *store, size_t size);

    /**
     * \brief Read-only blockstore constructor.
     *
     * \param store pointer to the memory region that will be used as a backend
     * \param size the size of the provided memory region
     */
    MuMemBlockStore(const void *store, size_t size);

    ~MuMemBlockStore() = default;
};
