/**
 * \file
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file LICENSE or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */

/// For 64-bit fseeko()/ftello(). Note: this is not portable outside of *nix platforms.
#define _FILE_OFFSET_BITS 64

#include "filestore.hh"

namespace MuStore {

void FileStore::close() {
    if (fh) {
        fclose(fh);
        fh = NULL;
    }
}

StoreError FileStore::seek(size_t lba) {
    if (!fh)
        return STORE_ERR_IO;
    if (lba >= blockCount)
        return STORE_ERR_OUT_OF_BOUNDS;

    if (fseeko(fh, (off_t)(lba * blockSize), SEEK_SET)) {
        close();
        return STORE_ERR_IO;
    }

    pos = lba;

    return STORE_ERR_OK;
}

StoreError FileStore::read(void *buffer) {
    if (!fh)
        return STORE_ERR_IO;
    if (pos >= blockCount)
        return STORE_ERR_OUT_OF_BOUNDS;

    if (fread(buffer, blockSize, 1, fh) != 1) {
        close();
        return STORE_ERR_IO;
    }

    pos++;

    return STORE_ERR_OK;
}

StoreError FileStore::write(const void *buffer) {
    if (!fh)
        return STORE_ERR_IO;
    if (pos >= blockCount)
        return STORE_ERR_OUT_OF_BOUNDS;
    if (!writable)
        return STORE_ERR_NOT_WRITABLE;

    if (fwrite(buffer, blockSize, 1, fh) != 1) {
        close();
        return STORE_ERR_IO;
    }

    pos++;

    return STORE_ERR_OK;
}

FileStore::FileStore(const char *path, bool writable_)
    : Store(512, 0, writable_) {

    if (writable_)
        fh = fopen(path, "r+b");
    else
        fh = fopen(path, "rb");

    if (fh) {
        off_t endPos;

        if (fseeko(fh, 0, SEEK_END)) {
            close();
            return;
        }
        if ((endPos = ftello(fh)) < 0) {
            close();
            return;
        }
        if (fseeko(fh, 0, SEEK_SET)) {
            close();
            return;
        }
        blockCount = (size_t)endPos / blockSize;
    }
}

FileStore::~FileStore() {
    if (fh)
        fclose(fh);
}

}
