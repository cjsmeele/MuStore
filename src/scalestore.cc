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
