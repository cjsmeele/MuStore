/**
 * \file
 * \brief     FileStore header.
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
