/**
 * \file
 * \brief     Tests for FatFs.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   Boost, see LICENSE
 */
#include "test.hh"
#include "fs.hh"

#include <filestore.hh>
#include <fatfs.hh>

TEST(fat_subtype) {
    auto store = FileStore(MUTEST_FAT16FILE);
    auto fs_   = FatFs(&store);
    ASSERT(fs_.getFsSubType() == FatFs::SubType::FAT16,
           "fat subtype must be FAT16, is %d", fs_.getFsSubType());
}

TEST_MAIN() {
    TEST_START();

    auto store = FileStore(MUTEST_FAT16FILE);
    LOG("bc: %lu", store.getBlockCount());

    TEST_FS_WITH(FatFs(&store), create);

    RUN_TEST(fat_subtype);

    TEST_FS_WITH(FatFs(&store), metadata);
    TEST_FS_WITH(FatFs(&store), root_readdir);
    TEST_FS_WITH(FatFs(&store), get_file);
    TEST_FS_WITH(FatFs(&store), get_dir);
    TEST_FS_WITH(FatFs(&store), file_read);
    TEST_FS_WITH(FatFs(&store), file_write);

    TEST_END();
}
