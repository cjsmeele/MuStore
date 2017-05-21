/**
 * \file
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file LICENSE or copy at
 * http://www.boost.org/LICENSE_1_0.txt
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
