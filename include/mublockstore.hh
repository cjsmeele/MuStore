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
 * Any non-zero value indicates an error.
 */
enum MuBlockStoreError : int {
    MUBLOCKSTORE_ERR_OK = 0,        ///< No error.
    MUBLOCKSTORE_ERR_IO,            ///< Generic I/O error.
    MUBLOCKSTORE_ERR_NOT_WRITABLE,  ///< Write to read-only medium attempted.
    MUBLOCKSTORE_ERR_OUT_OF_BOUNDS, ///< Attempted I/O operation exceeded medium size.
};

/**
 * \brief MuBlockStore generic block storage class.
 *
 * This class provides an interface for any type of storage that
 * supports seeking and reading / writing in chunks.
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
    /// \name Accessors
    /// @{

    /// Get the backend block size in bytes.
    size_t getBlockSize()  const { return blockSize;  }

    /// Get the amount of addressable blocks.
    size_t getBlockCount() const { return blockCount; }

    /// Check whether writes are permitted to this medium.
    bool   isWritable()    const { return writable;   }

    /// Get the current position as a LBA.
    size_t getPos()        const { return pos;        }

    /// @}

    /// \name I/O Operations
    /// @{

    /**
     * \brief Seek to the specified block address.
     *
     * \param lba The block number to seek to.
     *
     * \retval MUBLOCKSTORE_ERR_OK
     * \retval MUBLOCKSTORE_ERR_OUT_OF_BOUNDS when seeking to or past getBlockCount()
     * \retval MUBLOCKSTORE_ERR_IO for other backend errors
     */
    virtual MuBlockStoreError seek(size_t lba) = 0;

    /// Seek to the starting address.
    virtual MuBlockStoreError rewind() { return seek(0); }

    /**
     * \brief Read a single block from the current \ref pos "position"
     *        into the specified buffer.
     *
     * The \ref pos "position" is increased by one block after succesful completion.
     *
     * \warning The caller must make sure the buffer can hold at least
     * the amount of bytes specified by getBlockSize().
     *
     * \param buffer the destination buffer
     *
     * \retval MUBLOCKSTORE_ERR_OK
     * \retval MUBLOCKSTORE_ERR_OUT_OF_BOUNDS when reading past getBlockCount()
     * \retval MUBLOCKSTORE_ERR_IO for other backend errors
     */
    virtual MuBlockStoreError read (void *buffer) = 0;

    /**
     * \brief Write a single block from the specified buffer to the
     *        current \ref pos "position".
     *
     * The \ref pos "position" is increased by one block after succesful completion.
     *
     * \warning The caller must make sure the buffer holds at least
     * the amount of bytes specified by getBlockSize().
     *
     * \param buffer the source buffer
     *
     * \retval MUBLOCKSTORE_ERR_OK
     * \retval MUBLOCKSTORE_ERR_NOT_WRITABLE when attempting to write to a read-only medium (check isWritable() first)
     * \retval MUBLOCKSTORE_ERR_OUT_OF_BOUNDS when writing past getBlockCount()
     * \retval MUBLOCKSTORE_ERR_IO for other backend errors
     */
    virtual MuBlockStoreError write(const void *buffer) = 0;

    /// @}

    /// \name I/O Convenience Functions
    /// @{

    /// Shortcut for a seek() + read() operation. Returns the first error if any.
    virtual MuBlockStoreError read (size_t lba, void *buffer) {
        MuBlockStoreError err = seek(lba);
        if (!err)
            err = read(buffer);
        return err;
    }

    /// Shortcut for a seek() + write() operation. Returns the first error if any.
    virtual MuBlockStoreError write(size_t lba, const void *buffer) {
        MuBlockStoreError err = seek(lba);
        if (!err)
            err = write(buffer);
        return err;
    }

    /// @}

    MuBlockStore(size_t blockSize_ = 512, size_t blockCount_ = 0, bool writable_ = false)
        : blockSize(blockSize_),
          blockCount(blockCount_),
          writable(writable_),
          pos(0) { }

    virtual ~MuBlockStore() = default;
};
