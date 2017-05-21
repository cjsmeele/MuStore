/**
 * \file
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file LICENSE or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */
#include "scalestore.hh"

namespace MuStore {

StoreError ScaleStore::seek(size_t lba) {
    if (store && scale) {
        auto err = store->seek(lba * scale);
        if (!err)
            pos = lba;
        return err;
    } else {
        return STORE_ERR_IO;
    }
}

StoreError ScaleStore::read(void *buffer) {
    if (store && scale) {
        for (size_t i = 0; i < scale; i++) {
            auto err = store->read((uint8_t*)buffer + i * store->getBlockSize());
            if (err) {
                seek(pos); // Try to return.
                return err;
            }
        }
        pos++;
        return STORE_ERR_OK;
    } else {
        return STORE_ERR_IO;
    }
}

StoreError ScaleStore::write(const void *buffer) {
    if (store && scale) {
        for (size_t i = 0; i < scale; i++) {
            auto err = store->write((const uint8_t*)buffer + i * store->getBlockSize());
            if (err) {
                seek(pos); // Try to return.
                return err;
            }
        }
        pos++;
        return STORE_ERR_OK;
    } else {
        return STORE_ERR_IO;
    }
}

ScaleStore::ScaleStore(Store *store_, size_t blockSize_)
    : Store(
        blockSize_,
        store_->getBlockCount() / (blockSize_ / store_->getBlockSize())
      ),
      store(store_),
      scale(blockSize_ / store_->getBlockSize())
{
    if (blockSize % store->getBlockSize())
        store = nullptr; // Fail.
    else
        store->seek(0);
}

}
