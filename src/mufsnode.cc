/**
 * \file
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#include "mufsnode.hh"

MuFsNode MuFsNode::get(const char *path, MuFsError &err) {
    return fs->getChild(*this, path, err);
}

MuFsError MuFsNode::seek(size_t pos_) {
    return fs->seek(*this, pos_);
}

size_t MuFsNode::read(void *buffer, size_t size_, MuFsError &err) {
    return fs->read(*this, buffer, size_, err);
}

size_t MuFsNode::write(const void *buffer, size_t size_, MuFsError &err) {
    return fs->write(*this, buffer, size_, err);
}

MuFsNode MuFsNode::readDir(MuFsError &err) {
    return fs->readDir(*this, err);
}
