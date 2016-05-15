/**
 * \file
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#include "mufsnode.hh"

MuFsError MuFsNode::seek(size_t pos_) {
    return fs.seek(*this, pos_);
}

MuFsError MuFsNode::read (void *buffer, size_t size_) {
    return fs.read(*this, buffer, size_);
}

MuFsError MuFsNode::write(const void *buffer, size_t size_) {
    return fs.write(*this, buffer, size_);
}

MuFsNode MuFsNode::readDir(MuFsError &err) {
    return fs.readDir(*this, err);
}
