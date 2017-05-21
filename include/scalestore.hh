/**
 * \file
 * \brief     ScaleStore header.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file LICENSE or copy at
 * http://www.boost.org/LICENSE_1_0.txt
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
