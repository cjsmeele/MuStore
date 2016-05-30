/**
 * \file
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

}
