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
    auto store = MuFileBlockStore(MUTEST_FAT12FILE);
    auto fs_   = MuFatFs(store);
    ASSERT(fs_.getFsSubType() == MuFatFs::SubType::FAT12,
           "fat subtype must be FAT12, is %d", fs_.getFsSubType());
}

TEST(file_read_larger) {
    MuFsError err;
    FILE *fileRef = fopen("testfs/huge.txt", "r");
    ASSERT(fileRef, "fopen() failed: %s", strerror(errno));
    MuFsNode fileMu = fs->get("/huge.txt", err);
    ASSERT(!err, "get() of file '/huge.txt' failed (err=%d)", err);

    const size_t atATime = 59;
    char bufferMu[atATime]  = { };
    char bufferRef[atATime] = { };

    while (true) {
        size_t bytesReadRef = fread(bufferRef, 1, atATime, fileRef);
        size_t bytesReadMu  = fileMu.read(bufferMu, atATime, err);

        ASSERT(bytesReadRef == bytesReadMu,
               "fread and MuFsNode.read() didn't return the same amount of bytes (%lu vs %lu)",
               bytesReadRef, bytesReadMu);

        ASSERT(!memcmp(bufferRef, bufferMu, bytesReadRef),
               "read bytes from stdio and Mu are unequal");

        if (err == MUFS_EOF) {
            ASSERT(feof(fileRef), "unexpected EOF on read()");
            break;
        } else {
            ASSERT(!feof(fileRef), "expected EOF on read(), didn't get it");
        }
    }
}

TEST_MAIN() {
    TEST_START();

    auto store = MuFileBlockStore(MUTEST_FAT12FILE);
    LOG("bc: %lu", store.getBlockCount());

    TEST_FS_WITH(MuFatFs(store), create);

    RUN_TEST(fat_subtype);

    TEST_FS_WITH(MuFatFs(store), metadata);
    TEST_FS_WITH(MuFatFs(store), root_readdir);
    TEST_FS_WITH(MuFatFs(store), get_file);
    TEST_FS_WITH(MuFatFs(store), get_dir);
    TEST_FS_WITH(MuFatFs(store), file_read);
    TEST_FS_WITH(MuFatFs(store), file_read_larger);

    // WIP.

    TEST_END();
}
