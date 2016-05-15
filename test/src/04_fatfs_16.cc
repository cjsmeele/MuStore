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

TEST(fat_subtype) {
    auto store = MuFileBlockStore(MUTEST_FAT16FILE);
    auto fs_   = MuFatFs(store);
    ASSERT(fs_.getFsSubType() == MuFatFs::SubType::FAT16,
           "fat subtype must be FAT16, is %d", fs_.getFsSubType());
}

TEST_MAIN() {
    TEST_START();

    auto store = MuFileBlockStore(MUTEST_FAT16FILE);
    LOG("bc: %lu", store.getBlockCount());

    TEST_FS_WITH(MuFatFs(store), create);

    RUN_TEST(fat_subtype);

    TEST_FS_WITH(MuFatFs(store), metadata);
    TEST_FS_WITH(MuFatFs(store), root_readdir);
    //TEST_FS_WITH(MuFatFs(store), file_read);

    // WIP.

    TEST_END();
}
