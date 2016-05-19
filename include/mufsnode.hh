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

class MuFsNode {

    friend MuFs; ///< 😳  <3

public:
    static const size_t MAX_NAME_LENGTH = 32;
    static const size_t CONTEXT_SIZE    = 24;

protected:
    MuFs  &fs; ///< The MuFs in which this file resides.

    /**
     * \brief Fs private information.
     *
     * This may contain information such as the LBA of the file's
     * current position.
     */
    uint8_t fsContext[CONTEXT_SIZE] = { };

    bool   exists    = false;
    bool   directory = false;
    size_t size      = 0; ///< File size in bytes. This has no meaning for directories.
    size_t pos       = 0; ///< Byte index for files, directory index for directories.

    char   name[MAX_NAME_LENGTH+1] = { }; ///< Node basename.

public:
    bool doesExist()      const { return exists;    }
    bool isDirectory()    const { return directory; }
    size_t getSize()      const { return size;      }
    size_t getPos()       const { return pos;       }
    const char *getName() const { return name;      }

    // Proxy functions {{{
    MuFsNode get(const char *path, MuFsError &err);

    MuFsError seek(size_t pos_);
    MuFsError rewind() { return seek(0); };

    size_t read (void *buffer, size_t size, MuFsError &err);
    size_t write(const void *buffer, size_t size, MuFsError &err);

    MuFsNode readDir(MuFsError &err);
    // }}}

    MuFsNode(MuFs &fs_)
        : fs(fs_) { }

    virtual ~MuFsNode() = default;
};
