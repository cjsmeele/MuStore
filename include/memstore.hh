/**
 * \file
 * \brief     MemStore header.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 *
 * \page License
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
