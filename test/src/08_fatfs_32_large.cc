/**
 * \file
 * \brief     Tests for FatFs.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   Boost, see LICENSE
 */
#include "test.hh"
#include "fs.hh"

#include "filestore.hh"
#include "fatfs.hh"

TEST_MAIN() {
    TEST_START();

    auto store = FileStore(MUTEST_FAT32FILE_LARGE);

    TEST_FS_WITH(FatFs(&store), create);
    TEST_FS_WITH(FatFs(&store), metadata);
    TEST_FS_WITH(FatFs(&store), large_root_readdir);
    TEST_FS_WITH(FatFs(&store), large_file_read);

    TEST_END();
}
