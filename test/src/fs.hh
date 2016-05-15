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

// WIP.
