#include "test.hh"
#include "mufileblockstore.hh"

TEST(create) {
    MuFileBlockStore store(MUTEST_FAT12FILE);
    MuBlockStoreError err = store.seek(0);
    ASSERT(err == MUBLOCKSTORE_ERR_OK, "seek to start after creation (err=%d)", err);
}

TEST(seek) {
    MuFileBlockStore store(MUTEST_FAT12FILE);
    MuBlockStoreError err;

    LOG("Reported block size: %lu, block count: %lu",
        store.getBlockSize(), store.getBlockCount());

    err = store.seek(0);
    ASSERT(err == MUBLOCKSTORE_ERR_OK, "seek to start (err=%d)", err);
    size_t curPos = store.getPos();
    ASSERT(curPos == 0, "position 0 after seek to start (pos=%lu)", curPos);

    size_t endPos = store.getBlockCount() - 1;

    err = store.seek(endPos);
    ASSERT(err == MUBLOCKSTORE_ERR_OK, "seek to end (err=%d)", err);
    curPos = store.getPos();
    ASSERT(curPos == endPos, "position %lu after seek to end (pos=%lu)", endPos, curPos);

    err = store.seek(endPos + 1);
    ASSERT(err == MUBLOCKSTORE_ERR_OUT_OF_BOUNDS, "seek past end should fail");
}

TEST(read) {
    MuFileBlockStore store(MUTEST_FAT12FILE);
    ASSERT(false, "I don't feel like reading today");
}

TEST_MAIN() {
    TEST_START();

    LOG("Using <%s> as a FAT12 test file", MUTEST_FAT12FILE);
    LOG("Using <%s> as a FAT16 test file", MUTEST_FAT16FILE);
    LOG("Using <%s> as a FAT32 test file", MUTEST_FAT32FILE);

    RUN_TEST(create);
    RUN_TEST(seek);
    RUN_TEST(read);

    TEST_END();
}
