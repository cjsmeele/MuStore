/**
 * \file
 * \brief     Tests for Fs providers.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   Boost, see LICENSE
 */
#pragma once

#include "test.hh"
#include <fs.hh>

#include <string>
#include <list>
#include <vector>

using namespace MuStore;

Fs *fs;

#define TEST_FS_WITH(x, test)                 \
    {                                         \
        auto _fs = x;                         \
        fs = &_fs;                            \
        RUN_TEST(test);                       \
    }

TEST(create) {
    ASSERT(fs, "fs was not created");

    FsError err;
    auto dir = fs->getRoot(err);
    ASSERT(!err, "getRoot() failed (err=%d)", err);

    ASSERT(!strcmp(dir.getName(), "/"), "getRoot() failed (err=%d)", err);
}

TEST(metadata) {
    ASSERT(!strcmp(fs->getVolumeLabel(), "MUSTORETEST"),
           "volume label should be 'MUSTORETEST', is '%s'", fs->getVolumeLabel());
}

TEST(root_readdir) {
    FsError err;
    auto root = fs->getRoot(err);
    ASSERT(!err, "getRoot() failed (err=%d)", err);

    std::list<std::string> expectedEntries {
        "DIR1",
        "DIR2",
        "TEST.TXT",
        "HUGE.TXT",
        "WRITE.TXT",
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
            ASSERT(err == FS_EOF, "expected EOF on directory (err=%d)", err);
            break;
        }
    }
}

TEST(large_root_readdir) {
    std::vector<bool> foundDirs (false);
    foundDirs.resize(200);
    int entriesLeft = 200;

    FsError err;
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
            ASSERT(err == FS_EOF, "expected EOF on directory (err=%d)", err);
            break;
        }
    }
}

TEST(get_file) {
    FsError err;
    auto file = fs->get("/dir2/subsub/zstuff.txt", err);
    ASSERT(!err, "get() of file failed (err=%d)", err);
    ASSERT(file.doesExist(), "get() file does not exist");
    ASSERT(!file.isDirectory(), "get() file is a directory");
    ASSERT(file.getSize(), "get() file has no size");
    ASSERT(!strcmp(file.getName(), "ZSTUFF.TXT"),
           "name of file should be 'ZSTUFF.TXT', is '%s'", file.getName());
}

TEST(get_dir) {
    FsError err;
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

TEST(file_read) {
    FsError err;
    auto file = fs->get("/test.txt", err);
    ASSERT(!err, "get() of file '/test.txt' failed (err=%d)", err);

    const std::string expected = "Hello world\n"; // A 12 byte on disk.
    char buffer[32]  = { };
    char *p          = buffer;
    size_t totalRead = 0;
    const size_t atATime = 5;

    while (totalRead < expected.length()) {
        size_t bytesRead = file.read(p, atATime, err);
        if (err == FS_EOF) {
            ASSERT(bytesRead + totalRead == expected.length(),
                   "got unexpected EOF at %lu/%lu bytes",
                   bytesRead + totalRead, expected.length());
        } else {
            ASSERT(!err, "read() of file '/test.txt' failed (err=%d)", err);
        }

        if (bytesRead != atATime) {
            ASSERT(err == FS_EOF,
                   "read() returned %lu bytes instead of %lu",
                   bytesRead, atATime);
        }

        p         += bytesRead;
        totalRead += bytesRead;
    }

    ASSERT(!strncmp(expected.c_str(), buffer, expected.length()),
           "read string '%s' does not equal file contents",
          buffer);
}


TEST(large_file_read) {
    FsError err;
    FILE *fileRef = fopen("testfs_large/rtdir100/huge.txt", "r");
    ASSERT(fileRef, "fopen() failed: %s", strerror(errno));
    FsNode fileMu = fs->get("/rtdir100/huge.txt", err);
    ASSERT(!err, "get() of file '/rtdir100/huge.txt' failed (err=%d)", err);

    const size_t atATime = 13;
    char bufferMu[atATime]  = { };
    char bufferRef[atATime] = { };

    while (true) {
        size_t bytesReadRef = fread(bufferRef, 1, atATime, fileRef);
        size_t bytesReadMu  = fileMu.read(bufferMu, atATime, err);

        ASSERT(bytesReadRef == bytesReadMu,
               "fread and FsNode.read() didn't return the same amount of bytes (%lu vs %lu)",
               bytesReadRef, bytesReadMu);

        ASSERT(!memcmp(bufferRef, bufferMu, bytesReadRef),
               "read bytes from stdio and Mu are unequal");

        if (err == FS_EOF) {
            ASSERT(feof(fileRef), "unexpected EOF on read()");
            break;
        } else {
            ASSERT(!feof(fileRef), "expected EOF on read(), didn't get it");
        }
    }
}

TEST(file_write) {
    FsError err;
    FsNode file = fs->get("/write.txt", err);
    ASSERT(!err, "get() of file '/write.txt' failed (err=%d)", err);

    size_t currentSize = file.getSize();
    if (currentSize > 6) {
        file.seek(6);
        err = file.truncate();
        ASSERT(!err, "truncate failed with err=%d", err);
        file.rewind();
        currentSize = file.getSize();
    }
    ASSERT(currentSize == 6, "size of file '/write.txt' should be 6, is %lu", currentSize);

    uint8_t bufferR[4096 + 6] = { };

    size_t bytesRead = file.read(bufferR, 4096, err);
    ASSERT(bytesRead == 6, "read bytes should be 6, is %lu", bytesRead);
    ASSERT(!strcmp((char*)bufferR, "START\n"), "file content mismatch");

    uint8_t bufferW[4096];
    for (size_t i = 0; i < 4096; i++)
        bufferW[i] = rand();

    size_t bytesWritten = file.write(bufferW, 4096, err);
    ASSERT(!err, "write() of file '/write.txt' failed (err=%d)", err);
    ASSERT(bytesWritten == 4096, "written bytes should be 4096, is %lu", bytesWritten);

    currentSize = file.getSize();
    ASSERT(currentSize == 4096 + 6,
           "size of file '/write.txt' should be %lu, is %lu", 4096UL+6, currentSize);

    memset(bufferR, 0, sizeof(bufferR));
    err = file.rewind();
    ASSERT(!err, "rewind() of file '/write.txt' failed (err=%d)", err);

    bytesRead = file.read(bufferR, sizeof(bufferR), err);
    ASSERT(bytesRead == sizeof(bufferR),
           "read bytes should be %lu, is %lu", sizeof(bufferR), bytesRead);
    ASSERT(!strncmp((char*)bufferR, "START\n", 6), "file content mismatch (original text missing)");

    for (size_t i = 6; i < sizeof(bufferR); i++)
        ASSERT(bufferR[i] == bufferW[i-6], "file content mismatch at byte %lu", i);

    file.seek(6);
    file.truncate();
    ASSERT(file.getSize() == 6, "truncated file should be of size 6, is %lu", file.getSize());

    file.seek(2);
    file.write("E", 1, err);
    ASSERT(!err, "overwriting file failed with err=%d", err);
    file.rewind();
    memset(bufferR, 0, sizeof(bufferR));
    bytesRead = file.read(bufferR, 6, err);
    ASSERT(bytesRead == 6, "read bytes should be 6, is %lu", bytesRead);
    ASSERT(!strcmp((char*)bufferR, "STERT\n"), "file content mismatch (should be 'STERT', is '%6s')", bufferR);

    file.seek(2);
    file.write("A", 1, err);
}

TEST(file_remove) {
    ASSERT(false, "TEST WIP");
}

TEST(file_create) {
    ASSERT(false, "TEST WIP");
}

TEST(file_rename) {
    ASSERT(false, "TEST WIP");
}

TEST(file_move) {
    ASSERT(false, "TEST WIP");
}
