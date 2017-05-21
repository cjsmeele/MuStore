/**
 * \file
 * \brief     FsNode header.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file LICENSE or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */
#pragma once

#include "fs.hh"

namespace MuStore {

class Fs;
enum  FsError : int;

/**
 * \brief A file or directory in a MuFS filesystem.
 */
class FsNode {

    friend Fs; ///< 😳  <3

public:
    /// Max length of node basenames.
    static const size_t MAX_NAME_LENGTH = 32;

    /// Size of the node context region for FS implementation-defined usage.
    static const size_t CONTEXT_SIZE    = 40;

protected:
    Fs *fs; ///< The Fs in which this file resides.

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

    /// Proxy for Fs::get().
    FsNode get(const char *path, FsError &err);

    /// Proxy for Fs::seek().
    FsError seek(size_t pos_);

    /// Shortcut for seek().
    FsError rewind() { return seek(0); };

    /// Proxy for Fs::read().
    size_t read (void *buffer, size_t size, FsError &err);

    /// Proxy for Fs::write().
    size_t write(const void *buffer, size_t size, FsError &err);

    /// Proxy for Fs::readDir().
    FsNode readDir(FsError &err);

    /// Proxy for Fs::removeNode().
    FsError remove();

    /// Proxy for Fs::renameNode().
    FsError rename(const char *newName);

    /// Proxy for Fs::moveNode().
    FsError move(const char *newPath);

    /// Proxy for Fs::mkdir().
    FsNode mkdir(const char *name, FsError &err);

    /// Proxy for Fs::mkfile().
    FsNode mkfile(const char *name, FsError &err);

    /// Proxy for Fs::truncate().
    FsError truncate();

    /// @}

    FsNode(Fs *fs_)
        : fs(fs_) { }

    virtual ~FsNode() = default;
};

}
