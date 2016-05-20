/**
 * \file
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#include "mumemblockstore.hh"
#include <cstring>

MuBlockStoreError MuMemBlockStore::seek(size_t lba) {
    if (lba >= blockCount)
        return MUBLOCKSTORE_ERR_OUT_OF_BOUNDS;

    pos = lba;

    return MUBLOCKSTORE_ERR_OK;
}

MuBlockStoreError MuMemBlockStore::read(void *buffer) {
    if (pos >= blockCount)
        return MUBLOCKSTORE_ERR_OUT_OF_BOUNDS;

    memcpy(buffer, store+pos*blockSize, blockSize);

    pos++;

    return MUBLOCKSTORE_ERR_OK;
}

MuBlockStoreError MuMemBlockStore::write(const void *buffer) {
    if (pos >= blockCount)
        return MUBLOCKSTORE_ERR_OUT_OF_BOUNDS;
    if (!writable)
        return MUBLOCKSTORE_ERR_NOT_WRITABLE;

    memcpy(store+pos*blockSize, buffer, blockSize);

    pos++;

    return MUBLOCKSTORE_ERR_OK;
}

MuMemBlockStore::MuMemBlockStore(void *store_, size_t size)
    : MuBlockStore(512, size / 512, true),
      store((uint8_t*)store_)
{ }

MuMemBlockStore::MuMemBlockStore(const void *store_, size_t size)
    : MuBlockStore(512, size / 512, false),
      store((uint8_t*)store_)
{ }
