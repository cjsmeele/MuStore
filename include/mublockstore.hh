/**
 * \file
 * \brief     MuBlockStore header.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#pragma once

#include <cstdint>
#include <cstdlib>

/**
 * \brief MuBlockStore error numbers.
 *
 * Any non-zero value is an error.
 */
enum MuBlockStoreError {
    MUBLOCKSTORE_ERR_OK = 0,        ///< No error.
    MUBLOCKSTORE_ERR_IO,            ///< Generic I/O error.
    MUBLOCKSTORE_ERR_NOT_WRITABLE,  ///< Write to read-only medium attempted.
    MUBLOCKSTORE_ERR_OUT_OF_BOUNDS, ///< Attempted I/O operation exceeded medium size.
};

/**
 * \brief MuBlockStore generic block storage class.
 */
class MuBlockStore {

protected:
    // Properties of the medium used {{{
    size_t blockSize;  ///< The smallest size for all I/O operations in bytes.
    size_t blockCount; ///< Amount of blocks available on this medium.
    bool   writable;   ///< Whether this medium is writable.
    size_t pos;        ///< Current block number (LBA), incremented on read/write ops.
    // }}}

public:
    // Accessors {{{
    size_t getBlockSize()  const { return blockSize;  }
    size_t getBlockCount() const { return blockCount; }
    bool   isWritable()    const { return writable;   }
    size_t getPos()        const { return pos;        }
    // }}}

    // I/O {{{
    virtual MuBlockStoreError seek(size_t blockN) = 0;
    virtual MuBlockStoreError rewind() { return seek(0); }

    virtual MuBlockStoreError read (void *buffer) = 0;
    virtual MuBlockStoreError write(const void *buffer) = 0;

    /**
     * \brief Shortcut for a seek + read operation.
     */
    MuBlockStoreError read (size_t blockN, void *buffer) {
        MuBlockStoreError err = seek(blockN);
        if (!err)
            err = read(buffer);
        return err;
    }

    /**
     * \brief Shortcut for a seek + write operation.
     */
    MuBlockStoreError write(size_t blockN, const void *buffer) {
        MuBlockStoreError err = seek(blockN);
        if (!err)
            err = write(buffer);
        return err;
    }
    // }}}

    MuBlockStore(size_t blockSize_ = 512, size_t blockCount_ = 0, bool writable_ = false)
        : blockSize(blockSize_),
          blockCount(blockCount_),
          writable(writable_),
          pos(0) { }
    virtual ~MuBlockStore() = default;
};
