/**
 * \file
 * \brief     MuFsNode header.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#pragma once

#include "mufs.hh"

class MuFs;
enum  MuFsError : int;

/**
 * \brief A file or directory in a MuFS filesystem.
 */
class MuFsNode {

    friend MuFs; ///< 😳  <3

public:
    /// Max length of node basenames.
    static const size_t MAX_NAME_LENGTH = 32;

    /// Size of the node context region for FS implementation-defined usage.
    static const size_t CONTEXT_SIZE    = 24;

protected:
    MuFs *fs; ///< The MuFs in which this file resides.

    /**
     * \brief Fs private information.
     *
     * This may contain information such as the LBA of the file's
     * current position, or other FS-implementation dependant stuff.
     */
    uint8_t fsContext[CONTEXT_SIZE] = { };

    bool   exists    = false;
    bool   directory = false;
    size_t size      = 0; ///< File size in bytes. This has no meaning for directories.
    size_t pos       = 0; ///< Byte index for files, directory index for directories.

    char   name[MAX_NAME_LENGTH+1] = { }; ///< Node basename.

public:
    /// Check whether this node exists at all.
    bool doesExist()      const { return exists;    }

    /// Check whether this node is a directory.
    bool isDirectory()    const { return directory; }

    /// \brief Get the file node's size in bytes. Does not return anything
    /// useful for directories, unless you're a fan of \0 bytes.
    size_t getSize()      const { return size;      }

    /// Get the current position in bytes.
    size_t getPos()       const { return pos;       }

    /// Get the node's basename.
    const char *getName() const { return name;      }

    /// \name Proxy functions
    /// @{

    /// Proxy for MuFs::get().
    MuFsNode get(const char *path, MuFsError &err);

    /// Proxy for MuFs::seek().
    MuFsError seek(size_t pos_);

    /// Shortcut for seek().
    MuFsError rewind() { return seek(0); };

    /// Proxy for MuFs::read().
    size_t read (void *buffer, size_t size, MuFsError &err);

    /// Proxy for MuFs::write().
    size_t write(const void *buffer, size_t size, MuFsError &err);

    /// Proxy for MuFs::readDir().
    MuFsNode readDir(MuFsError &err);

    /// @}

    MuFsNode(MuFs *fs_)
        : fs(fs_) { }

    virtual ~MuFsNode() = default;
};
