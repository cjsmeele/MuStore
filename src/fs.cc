/**
 * \file
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file LICENSE or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */
#include "fs.hh"
#include <cstring>

namespace MuStore {

FsNode Fs::makeNode(
    const char *name,
    bool exists,
    bool directory,
    size_t size
) {
    FsNode node {this};
    strncpy(node.name, name, node.MAX_NAME_LENGTH);
    node.exists    = exists;
    node.directory = directory;
    node.size      = size;

    return node;
}

void Fs::nodeUpdatePos(FsNode &node, size_t pos) const {
    node.pos = pos;
}
void Fs::nodeUpdateSize(FsNode &node, size_t size) const {
    node.size = size;
}

void *Fs::getNodeContext(FsNode &node) const {
    return node.fsContext;
}

#if !defined(_DEFAULT_SOURCE) && !defined(_BSD_SOURCE)
// There is no strncasecmp function defined on this platform, we will
// provide our own.

inline static char _lc(char c) {
    return (char)(c >= 'A' && c <= 'Z'
                  ? c + 'a' - 'A'
                  : c);
}

static int strncasecmp(const char *s1, const char *s2, size_t n) throw() {

    for (size_t i = 0; i < n; i++, s1++, s2++) {
        char a = _lc(*s1);
        char b = _lc(*s2);

        if (a != b)
            return a < b ? -1 : 1;
        if (!a)
            break;
    }
    return 0;
}

#endif /* !defined(_DEFAULT_SOURCE) && !defined(_BSD_SOURCE) */

FsNode Fs::getChild(FsNode &root, const char *path, FsError &err) {

    if (!root.isDirectory()) {
        err = FS_ERR_NOT_DIRECTORY;
        // Return a new non-existent node on failure.
        return {this};
    }

    // Trim leading slashes.
    while (path[0] == '/')
        path++;

    if (strlen(path) == 0) {
        err = FS_ERR_OK;
        return root;
    }

    const char *nextPart = strchr(path, '/');
    if (!nextPart)
        nextPart = path + strlen(path);

    size_t partLength = (size_t)(nextPart - path);

    // The name of the direct descendant node we're looking for is now
    // path[0..^partLength].

    err = root.rewind();
    if (err)
        return {this};

    while (true) {
        FsNode child = readDir(root, err);

        if (err) {
            if (err == FS_EOF)
                err = FS_ERR_OBJECT_NOT_FOUND;

            root.rewind();
            return {this};
        }
        if (strlen(child.getName()) == partLength) {
            if (
                (this->isCaseSensitive() && !strncmp(child.getName(), path, partLength))
                ||
                (!this->isCaseSensitive() && !strncasecmp(child.getName(), path, partLength))
            ) {
                // Hebbes. :D

                root.rewind();

                if (strlen(nextPart)) {
                    if (child.isDirectory()) {
                        // Gotta go deeper.
                        return getChild(child, nextPart, err);
                    } else {
                        err = FS_ERR_OBJECT_NOT_FOUND;
                        return {this};
                    }
                } else {
                    err = FS_ERR_OK;
                    return child;
                }
            }
        }
    }
}

FsNode Fs::get(const char *path, FsError &err) {

    FsNode root = getRoot(err);
    if (err)
        return {this};

    return getChild(root, path, err);
}

}
