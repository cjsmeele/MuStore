/**
 * \file
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#include "muscaleblockstore.hh"

MuBlockStoreError MuScaleBlockStore::seek(size_t lba) {
    if (store && scale) {
        auto err = store->seek(lba * scale);
        if (!err)
            pos = lba;
        return err;
    } else {
        return MUBLOCKSTORE_ERR_IO;
    }
}

MuBlockStoreError MuScaleBlockStore::read(void *buffer) {
    if (store && scale) {
        for (size_t i = 0; i < scale; i++) {
            auto err = store->read((uint8_t*)buffer + i * store->getBlockSize());
            if (err) {
                seek(pos); // Try to return.
                return err;
            }
        }
        pos++;
        return MUBLOCKSTORE_ERR_OK;
    } else {
        return MUBLOCKSTORE_ERR_IO;
    }
}

MuBlockStoreError MuScaleBlockStore::write(const void *buffer) {
    if (store && scale) {
        for (size_t i = 0; i < scale; i++) {
            auto err = store->write((const uint8_t*)buffer + i * store->getBlockSize());
            if (err) {
                seek(pos); // Try to return.
                return err;
            }
        }
        pos++;
        return MUBLOCKSTORE_ERR_OK;
    } else {
        return MUBLOCKSTORE_ERR_IO;
    }
}

MuScaleBlockStore::MuScaleBlockStore(MuBlockStore *store_, size_t blockSize_)
    : MuBlockStore(
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
