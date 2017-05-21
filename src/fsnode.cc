/**
 * \file
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 *
 * Distributed under the Boost Software License, Version 1.0.
 * See accompanying file LICENSE or copy at
 * http://www.boost.org/LICENSE_1_0.txt
 */
#include "fsnode.hh"

namespace MuStore {

FsNode FsNode::get(const char *path, FsError &err) {
    return fs->getChild(*this, path, err);
}

FsError FsNode::seek(size_t pos_) {
    return fs->seek(*this, pos_);
}

size_t FsNode::read(void *buffer, size_t size_, FsError &err) {
    return fs->read(*this, buffer, size_, err);
}

size_t FsNode::write(const void *buffer, size_t size_, FsError &err) {
    return fs->write(*this, buffer, size_, err);
}

FsNode FsNode::readDir(FsError &err) {
    return fs->readDir(*this, err);
}

FsError FsNode::remove() {
    return fs->removeNode(*this);
}

FsError FsNode::rename(const char *newName) {
    return fs->renameNode(*this, newName);
}

FsError FsNode::move(const char *newPath) {
    return fs->moveNode(*this, newPath);
}

FsNode FsNode::mkdir(const char *name_, FsError &err) {
    return fs->mkdir(*this, name_, err);
}

FsNode FsNode::mkfile(const char *name_, FsError &err) {
    return fs->mkfile(*this, name_, err);
}

FsError FsNode::truncate() {
    return fs->truncate(*this);
}

}
