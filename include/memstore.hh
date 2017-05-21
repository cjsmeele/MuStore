/**
 * \file
 * \brief     MemStore header.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file LICENSE or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */
#pragma once

#include "store.hh"

namespace MuStore {

/**
 * \brief Store memory backend.
 *
 * This allows any contiguous memory region to be approached as a
 * Store.
 */
class MemStore : public Store {

private:
    /// The store, represented as a memory region.
    const uint8_t *roStore;
    /// Writable store, will be `nullptr` for ro memory.
          uint8_t   *store;

public:
    StoreError seek(size_t lba);

    StoreError read (void *buffer);
    StoreError write(const void *buffer);

    using Store::read;
    using Store::write;

    /**
     * \brief Writable blockstore constructor.
     *
     * \param store pointer to the memory region that will be used as a backend
     * \param size the size of the provided memory region
     */
    MemStore(void *store, size_t size);

    /**
     * \brief Read-only blockstore constructor.
     *
     * \param store pointer to the memory region that will be used as a backend
     * \param size the size of the provided memory region
     */
    MemStore(const void *store, size_t size);

    ~MemStore() = default;
};

}
