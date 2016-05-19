/**
 * \file
 * \brief     Tests for MuFatFs.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#include "test.hh"
#include "fs.hh"

#include "mufileblockstore.hh"
#include "mufatfs.hh"

TEST_MAIN() {
    TEST_START();

    auto store = MuFileBlockStore(MUTEST_FAT16FILE_LARGE);

    TEST_FS_WITH(MuFatFs(store), create);
    TEST_FS_WITH(MuFatFs(store), metadata);
    TEST_FS_WITH(MuFatFs(store), large_root_readdir);
    TEST_FS_WITH(MuFatFs(store), large_file_read);

    // WIP.

    TEST_END();
}
