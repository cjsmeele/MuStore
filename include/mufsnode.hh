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

protected:
    MuFs  &fs;
    bool   exists    = false;
    bool   directory = false;
    size_t size      = 0;
    size_t pos       = 0; ///< Byte index for files, directory index for directories.

    char   name[MAX_NAME_LENGTH+1] = { };

public:
    bool doesExist()      const { return exists;    }
    bool isDirectory()    const { return directory; }
    bool getSize()        const { return size;      }
    bool getPos()         const { return pos;       }
    const char *getName() const { return name;      }

    MuFsError seek(size_t pos_);
    MuFsError rewind() { return seek(0); };

    MuFsError read (void *buffer, size_t size_);
    MuFsError write(const void *buffer, size_t size_);

    MuFsNode readDir(MuFsError &err);

    MuFsNode(MuFs &fs_)
        : fs(fs_) { }

    virtual ~MuFsNode() = default;
};
