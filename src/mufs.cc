/**
 * \file
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#include "mufs.hh"
#include <cstring>

MuFsNode MuFs::makeNode(
    const char *name,
    bool exists,
    bool directory
) {
    MuFsNode node {*this};
    strncpy(node.name, name, node.MAX_NAME_LENGTH);
    node.exists    = exists;
    node.directory = directory;

    return node;
}

MuFsNode MuFs::getChild(MuFsNode &root, const char *path, MuFsError &err) {

    // Trim leading slashes.
    while (path[0] == '/')
        path++;

    if (strlen(path) == 0)
        return root;

    const char *nextPart = strchr(path, '/');
    if (!nextPart)
        nextPart = path + strlen(path);

    size_t partLength = (size_t)(nextPart - path);

    // The name of the direct descendant node we're looking for is now
    // path[0..^partLength].

    err = root.rewind();
    if (err) {
        // Return a new non-existant node on failure.
        return {*this};
    }

    while (true) {
        MuFsNode child = readDir(root, err);

        if (err) {
            if (err == MUFS_EOF)
                err = MUFS_ERR_OBJECT_NOT_FOUND;

            root.rewind();
            return {*this};
        }
        if (strlen(child.getName()) == partLength
            && !strncmp(child.getName(), path, partLength)) {
            // Hebbes. :D

            root.rewind();

            if (strlen(nextPart)) {
                // Gotta go deeper.
                // Note: `child` may not be a directory - that's OK,
                // it will be catched in the next call.
                return getChild(child, nextPart, err);
            } else {
                return child;
            }
        }
    }
}

MuFsNode MuFs::get(const char *path, MuFsError &err) {

    MuFsNode root = getRoot(err);
    if (err)
        return {*this};

    return getChild(root, path, err);
}
