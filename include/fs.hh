/**
 * \file
 * \brief     Fs header.
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
#include "fsnode.hh"

namespace MuStore {

class FsNode;

/**
 * \brief Fs error numbers.
 *
 * Any non-zero value indicates an error.
 *
 * \note A storage size is specified to allow for forward declaration.
 * \note An enum class was avoided to allow for more convenient /
 *       intuitive error checking (i.e.: `if (err) {}`).
 */
enum FsError : int {
    FS_ERR_OK = 0,            ///< No error.
    FS_ERR_IO,                ///< An error in the I/O (Store) layer.
    FS_ERR_NO_SPACE,          ///< No storage space available for attempted file operation.
    FS_ERR_NOT_FILE,          ///< File operation attempted on non-file object.
    FS_ERR_NOT_DIRECTORY,     ///< Directory operation attempted on non-directory object.
    FS_ERR_OBJECT_NOT_FOUND,  ///< Referenced filesystem object (file/dir) does not exist.
    FS_ERR_OPER_UNAVAILABLE,  ///< Attempted operation is not available for this filesystem.
    FS_EOF,                   ///< The end of a file or directory was already reached.
};

/**
 * \brief Fs generic filesystem interface.
 */
class Fs {

protected:
    Store *store;               ///< The underlying block storage.
    char volumeLabel[33] = { }; ///< A label describing this volume.

    /**
     * \brief A node factory for our subclasses.
     *
     * \param name a node basename (not an absolute path).
     *        A copy of this string will be stored in the node. If the
     *        name exceeds FsNode::MAX_NAME_LENGTH, it will be
     *        trimmed.
     * \param exists
     * \param directory whether this is a directory
     * \param size a file size in bytes (not applicable for directories)
     */
    FsNode makeNode(
        const char *name,
        bool exists,
        bool directory,
        size_t size = 0
    );

    /// Update the position of the provided node.
    void nodeUpdatePos(FsNode &node, size_t newPos) const;

    /// Get a pointer to the nodeContext member of the provided node.
    void *getNodeContext(FsNode &node) const;

public:
    /// Get a label describing this filesystem. May be an empty string.
            const char *getVolumeLabel() const { return volumeLabel; }

    /// Get the type of FS implementation used (e.g. "FAT").
    virtual const char *getFsType()      const = 0;

    /// \brief Check whether filenames are case-sensitive in this filesystem.
    ///
    /// If false, get() and getChild() will behave accordingly.
    virtual bool isCaseSensitive()       const { return true; }

    /// \todo createFile, createDir, getParent, move, delete, ...

    /// \name Directory operations
    /// @{

    /**
     * \brief Get the root node of this filesystem.
     *
     * The returned node will be a directory.
     *
     * \param[out] err one of:
     * - \ref FS_ERR_OK
     * - \ref FS_ERR_IO
     * - \ref FS_ERR_OPER_UNAVAILABLE
     */
    virtual FsNode getRoot(FsError &err) = 0;

    /**
     * \brief Recursively get a node starting at the given directory.
     *
     * \param[in]  root the directory node from which to search
     * \param[in]  path a path relative to the given root node.
     *             Note: `..` and `.` will not work in paths
     * \param[out] err one of:
     *   - \ref FS_ERR_OK
     *   - \ref FS_ERR_IO
     *   - \ref FS_ERR_OPER_UNAVAILABLE
     *   - \ref FS_ERR_NOT_DIRECTORY
     *   - \ref FS_ERR_OBJECT_NOT_FOUND
     *
     * \return the queried child node if successful, a non-existent node otherwise (check `err`).
     *
     * \sa get()
     */
    virtual FsNode getChild(FsNode &root, const char *path, FsError &err);

    /**
     * \brief Recursively get a node using an absolute path.
     *
     * \param[in]  path an absolute path (`..` and `.` will not work here)
     * \param[out] err one of:
     *   - \ref FS_ERR_OK
     *   - \ref FS_ERR_IO
     *   - \ref FS_ERR_OPER_UNAVAILABLE
     *   - \ref FS_ERR_NOT_DIRECTORY
     *   - \ref FS_ERR_OBJECT_NOT_FOUND
     *
     * \return the queried node if successful, a non-existent node otherwise (check `err`).
     *
     * \sa getChild()
     */
    virtual FsNode get(const char *path, FsError &err);

    /**
     * \brief Read the next node entry from a directory.
     *
     * \param[in]  parent the directory to read
     * \param[out] err one of:
     *   - \ref FS_ERR_OK
     *   - \ref FS_ERR_IO
     *   - \ref FS_ERR_OPER_UNAVAILABLE
     *   - \ref FS_ERR_OBJECT_NOT_FOUND
     *   - \ref FS_ERR_NOT_DIRECTORY
     *
     * \return a child node if successful, a non-existent node otherwise (check `err`).
     */
    virtual FsNode readDir(FsNode &parent, FsError &err) = 0;

    virtual FsError removeNode(FsNode &node) = 0;

    virtual FsError renameNode(FsNode &node, const char *newName) = 0;

    virtual FsError moveNode(FsNode &node, const char *newPath) = 0;

    virtual FsNode mkdir(FsNode &parent, const char *name, FsError &err) = 0;

    virtual FsNode mkfile(FsNode &parent, const char *name, FsError &err) = 0;

    /// @}

    /// \name File I/O
    /// @{

    /**
     * \brief Seek in a file or directory.
     *
     * \param node the node to seek in, can be either a file or directory
     * \param pos_ the position to seek to. A byte number for files, a directory index number for directories.
     *
     * \retval FS_ERR_OK
     * \retval FS_ERR_IO
     * \retval FS_ERR_OPER_UNAVAILABLE
     * \retval FS_ERR_OBJECT_NOT_FOUND
     */
    virtual FsError seek(FsNode &node, size_t pos_) = 0;

    /**
     * \brief Read some bytes from a file node.
     *
     * \param[in]    file the file to read from
     * \param[inout] buffer the buffer to read into
     * \param[in]    size the amount of bytes to read
     * \param[out]   err one of:
     *   - \ref FS_ERR_OK
     *   - \ref FS_ERR_IO
     *   - \ref FS_EOF
     *   - \ref FS_ERR_OPER_UNAVAILABLE
     *   - \ref FS_ERR_OBJECT_NOT_FOUND
     *   - \ref FS_ERR_NOT_FILE
     *
     * \return the amount of bytes actually read. Will be lower than
     *         `size` on error or EOF, check `err`!
     */
    virtual size_t read (FsNode &file, void *buffer, size_t size, FsError &err) = 0;

    /**
     * \brief Write some bytes to a file node.
     *
     * \param[in]   file the file to write to
     * \param[in]   buffer the buffer to write from
     * \param[in]   size the amount of bytes to write
     * \param[out]  err one of:
     *   - \ref FS_ERR_OK
     *   - \ref FS_ERR_IO
     *   - \ref FS_ERR_NO_SPACE
     *   - \ref FS_ERR_OPER_UNAVAILABLE
     *   - \ref FS_ERR_OBJECT_NOT_FOUND
     *   - \ref FS_ERR_NOT_FILE
     *
     * \return the amount of bytes actually written. Will be lower
     *         than `size` on error, check `err`!
     */
    virtual size_t write(FsNode &file, const void *buffer, size_t size, FsError &err) = 0;

    /// @}

    /**
     * \brief Fs constructor.
     *
     * A Fs needs a block storage backend.
     *
     * \param store_ the storage backend to use for this filesystem
     */
    Fs(Store *store_)
        : store(store_) { }

    virtual ~Fs() = default;
};

}
