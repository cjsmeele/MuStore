/**
 * \file
 * \brief     Tests for Fs providers.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#pragma once

#include "test.hh"
#include "mufs.hh"

#include <string>
#include <list>
#include <vector>

MuFs *fs;

#define TEST_FS_WITH(x, test)                 \
    {                                         \
        auto _fs = x;                         \
        fs = &_fs;                            \
        RUN_TEST(test);                       \
    }

TEST(create) {
    ASSERT(fs, "fs was not created");

    MuFsError err;
    auto dir = fs->getRoot(err);
    ASSERT(!err, "getRoot() failed (err=%d)", err);

    ASSERT(!strcmp(dir.getName(), "/"), "getRoot() failed (err=%d)", err);
}

TEST(metadata) {
    ASSERT(!strcmp(fs->getVolumeLabel(), "MUSTORETEST"),
           "volume label should be 'MUSTORETEST', is '%s'", fs->getVolumeLabel());
}

TEST(node_get) {
    MuFsError err;
    auto dir = fs->getRoot(err);
    ASSERT(!err, "getRoot() failed (err=%d)", err);

    // WIP.
}

TEST(file_read) {
    MuFsError err;
    auto dir = fs->getRoot(err);
    ASSERT(!err, "getRoot() failed (err=%d)", err);

    // WIP.
}

TEST(root_readdir) {
    MuFsError err;
    auto root = fs->getRoot(err);
    ASSERT(!err, "getRoot() failed (err=%d)", err);

    std::list<std::string> expectedEntries {
        "DIR1",
        "DIR2",
        "TEST.TXT",
    };

    size_t i = 0;
    while (1) {
        ASSERT(++i < 1000, "got stuck in an infinite loop reading a directory");
        auto child = root.readDir(err);
        LOG("Got dirent '%s'", child.getName());

        if (expectedEntries.size()) {
            ASSERT(!err, "readDir() failed (err=%d)", err);
            ASSERT(child.doesExist(), "readDir() returned non-existant entry without error");

            auto it = expectedEntries.begin();
            bool found = false;
            while (it != expectedEntries.end()) {
                if (*it == child.getName()) {
                    if (strncmp(child.getName(), "DIR", 3) == 0) {
                        ASSERT(child.isDirectory(),
                               "test dir <%s> is not a directory", child.getName());
                    } else {
                        ASSERT(!child.isDirectory(),
                               "test file <%s> is a directory", child.getName());
                    }

                    it = expectedEntries.erase(it);
                    found = true;
                    break;
                } else {
                    it++;
                }
            }
            ASSERT(found, "unexpected direntry '%s'", child.getName());
        } else {
            ASSERT(err == MUFS_EOF, "expected EOF on directory (err=%d)", err);
            break;
        }
    }
}

TEST(large_root_readdir) {
    std::vector<bool> foundDirs (false);
    foundDirs.resize(200);
    int entriesLeft = 200;

    MuFsError err;
    auto root = fs->getRoot(err);
    ASSERT(!err, "getRoot() failed (err=%d)", err);

    while (1) {
        auto child = root.readDir(err);

        if (!err && !strcmp(child.getName(), "GENFILES.PL"))
            continue;

        if (entriesLeft) {
            ASSERT(!err, "readDir() failed (err=%d)", err);
            ASSERT(child.doesExist(), "readDir() returned non-existant entry without error");

            int dirNum = 0;
            ASSERT(sscanf(child.getName(), "RTDIR%04d", &dirNum) == 1,
                   "unexpected direntry '%s'", child.getName());
            ASSERT(dirNum >= 1 && dirNum <= 200,
                   "unexpected direntry '%s'", child.getName());

            ASSERT(!foundDirs[dirNum], "duplicate entry for '%s'", child.getName());

            foundDirs[dirNum] = true;
            entriesLeft--;

        } else {
            ASSERT(err == MUFS_EOF, "expected EOF on directory (err=%d)", err);
            break;
        }
    }
}

TEST(get_file) {
    MuFsError err;
    auto file = fs->get("/dir2/subsub/zstuff.txt", err);
    ASSERT(!err, "get() of file failed (err=%d)", err);
    ASSERT(file.doesExist(), "get() file does not exist");
    ASSERT(!file.isDirectory(), "get() file is a directory");
    ASSERT(file.getSize(), "get() file has no size");
    ASSERT(!strcmp(file.getName(), "ZSTUFF.TXT"),
           "name of file should be 'ZSTUFF.TXT', is '%s'", file.getName());
}

TEST(get_dir) {
    MuFsError err;
    auto dir = fs->get("/dir2/subsub", err);
    ASSERT(!err, "get() of directory failed (err=%d)", err);
    ASSERT(dir.doesExist(), "get() directory does not exist");
    ASSERT(dir.isDirectory(), "get() directory is not a directory");
    ASSERT(!strcmp(dir.getName(), "SUBSUB"),
           "name of dir should be 'SUBSUB', is '%s'", dir.getName());

    auto file = dir.get("stuff.txt", err);
    ASSERT(!err, "get() within directory failed (err=%d)", err);
    ASSERT(file.doesExist(), "get() file in directory does not exist");

    auto child = dir.readDir(err);
    ASSERT(!err, "readDir() on get() directory failed (err=%d)", err);
    ASSERT(child.doesExist(), "readDir() on get() directory returned non-existent child");
}

// TEST(large_file_read) {
//     ASSERT(false, "unimplemented");
// }
