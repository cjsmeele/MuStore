/**
 * \file
 * \brief     FileStore header.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file LICENSE or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */
#pragma once

#include "store.hh"
#include <cstdio>

namespace MuStore {

/**
 * \brief Store file backend.
 *
 * This uses a single file as a block storage backend.
 */
class FileStore : public Store {

private:
    /// The storage backend is a stdio file handle.
    FILE *fh;

    /**
     * \brief Close the file, disallow any further I/O operations.
     *
     * Use this for unrecoverable errors.
     */
    void close();

public:
    StoreError seek(size_t lba);

    StoreError read (void *buffer);
    StoreError write(const void *buffer);

    // Needed because we overload read and write methods.
    using Store::read;
    using Store::write;

    /**
     * \param path path to the file that will be used as a storage backend
     * \param writable whether to allow write access to the file
     */
    FileStore(const char *path, bool writable = true);
    ~FileStore();
};

}
