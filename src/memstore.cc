/**
 * \file
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
#include "memstore.hh"
#include <cstring>

namespace MuStore {

StoreError MemStore::seek(size_t lba) {
    if (lba >= blockCount)
        return STORE_ERR_OUT_OF_BOUNDS;

    pos = lba;

    return STORE_ERR_OK;
}

StoreError MemStore::read(void *buffer) {
    if (pos >= blockCount)
        return STORE_ERR_OUT_OF_BOUNDS;

    memcpy(buffer, roStore+pos*blockSize, blockSize);

    pos++;

    return STORE_ERR_OK;
}

StoreError MemStore::write(const void *buffer) {
    if (pos >= blockCount)
        return STORE_ERR_OUT_OF_BOUNDS;
    if (!writable || !store)
        return STORE_ERR_NOT_WRITABLE;

    memcpy(store+pos*blockSize, buffer, blockSize);

    pos++;

    return STORE_ERR_OK;
}

MemStore::MemStore(void *store_, size_t size)
    : Store(512, size / 512, true),
      roStore((uint8_t*)store_),
        store((uint8_t*)store_)
{ }

MemStore::MemStore(const void *store_, size_t size)
    : Store(512, size / 512, false),
      roStore((const uint8_t*)store_),
        store(nullptr)
{ }

}
