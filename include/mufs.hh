/**
 * \file
 * \brief     MuFs header.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#pragma once

#include "mublockstore.hh"
#include "mufsnode.hh"

class MuFsNode;

/**
 * \brief MuFs error numbers.
 *
 * Any non-zero value indicates an error.
 *
 * \note A storage size is specified to allow for forward declaration.
 * \note An enum class was avoided to allow for more convenient /
 *       intuitive error checking (i.e.: `if (err) {}`).
 */
enum MuFsError : int {
    MUFS_ERR_OK = 0,            ///< No error.
    MUFS_ERR_IO,                ///< An error in the I/O (MuBlockStore) layer.
    MUFS_ERR_NO_SPACE,          ///< No storage space available for attempted file operation.
    MUFS_ERR_NOT_FILE,          ///< File operation attempted on non-file object.
    MUFS_ERR_NOT_DIRECTORY,     ///< Directory operation attempted on non-directory object.
    MUFS_ERR_OBJECT_NOT_FOUND,  ///< Referenced filesystem object (file/dir) does not exist.
    MUFS_ERR_OPER_UNAVAILABLE,  ///< Attempted operation is not available for this filesystem.
    MUFS_EOF,                   ///< The end of a file or directory was already reached.
};

/**
 * \brief MuFs generic filesystem interface.
 */
class MuFs {

protected:
    MuBlockStore &store;        ///< The underlying block storage.
    char volumeLabel[33] = { }; ///< A label describing this volume.

    /**
     * \brief A node factory for our subclasses.
     *
     * We act as a trusted party of MuFsNode to them (actually me and
     * MuFsNode are BFFs, but don't tell anyone ;).
     */
    MuFsNode makeNode(
        const char *name,
        bool exists,
        bool directory,
        size_t size = 0
    );

    void nodeUpdatePos(MuFsNode &node, size_t newPos) const;

    void *getNodeContext(MuFsNode &node) const;

public:
            const char *getVolumeLabel() const { return volumeLabel; }
    virtual const char *getFsType()      const = 0;
    virtual bool isCaseSensitive()       const { return true; }

    // Directory operations {{{
    virtual MuFsNode getRoot(MuFsError &err) = 0;
    virtual MuFsNode getChild(MuFsNode &root, const char *path, MuFsError &err);
    virtual MuFsNode get(const char *path, MuFsError &err);
    virtual MuFsNode readDir(MuFsNode &parent, MuFsError &err) = 0;
    // }}}

    // TODO: createDir, deleteDir, createFile, deleteFile, move, getParent...

    // File I/O {{{
    virtual MuFsError seek (MuFsNode &file, size_t pos_) = 0;
    virtual size_t read (MuFsNode &file,       void *buffer, size_t size, MuFsError &err) = 0;
    virtual size_t write(MuFsNode &file, const void *buffer, size_t size, MuFsError &err) = 0;
    // }}}

    MuFs(MuBlockStore &store_)
        : store(store_) { }
    virtual ~MuFs() = default;
};
