/**
 * \file
 * \brief     ScaleStore header.
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
 * \brief Store decorator for upscaling block sizes.
 *
 * This allows one to access stores with smaller block sizes as if
 * they have a larger block size. A single read or write call results
 * in multiple operations of that type on the underlying store.
 */
class ScaleStore : public Store {

private:
    /// The store we pass calls to.
    Store *store;

    /// Block size scale. The block size of this store must be a
    /// multiple of that of the underlying block store.
    size_t scale;

public:
    StoreError seek(size_t lba);

    StoreError read (void *buffer);
    StoreError write(const void *buffer);

    using Store::read;
    using Store::write;

    ScaleStore(Store *store_, size_t blockSize);

    ~ScaleStore() = default;
};

}
