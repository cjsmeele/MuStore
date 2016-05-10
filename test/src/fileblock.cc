#include "test.hh"
#include "mufileblockstore.hh"

#include <ctime>

TEST(create) {
    MuFileBlockStore store(MUTEST_FAT12FILE);
    ASSERT(store.getPos() == 0, "initial pos must be zero (pos=%lu)", store.getPos());

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
    MuBlockStoreError err;

    uint8_t buffer[4096];
    ASSERT(store.getBlockSize() >= 512,  "block size too small");
    ASSERT(store.getBlockSize() <= 4096, "can't test, block size too large");

    err = store.read(buffer);

    ASSERT(err == MUBLOCKSTORE_ERR_OK, "read block (err=%d)", err);

    uint16_t sig = ((uint16_t)buffer[510] << 8) | buffer[511];
    ASSERT(sig == 0x55aa, "boot sector sig mismatch on read (%#04x != %#04x)", sig, 0x55aa);

    ASSERT(store.getPos() == 1, "pos should be 1 after reading first block (pos=%lu)", store.getPos());
}

TEST(write) {
    MuFileBlockStore store(MUTEST_FAT12FILE);
    MuBlockStoreError err;

    ASSERT(store.getBlockSize() >= 512,  "block size too small");
    ASSERT(store.getBlockSize() <= 4096, "can't test, block size too large");

    uint8_t buffer1[4096];
    uint8_t buffer2[4096] = { };

    srand(time(NULL));

    for (size_t i = 0; i < store.getBlockSize(); i++)
        buffer1[i] = rand();

    err = store.write(store.getBlockCount()-1, buffer1);

    ASSERT(err == MUBLOCKSTORE_ERR_OK, "write block (err=%d)", err);
    ASSERT(store.getPos() == store.getBlockCount(),
           "pos should be %lu after writing last block (pos=%lu)",
           store.getBlockCount(), store.getPos());

    ASSERT(store.read(buffer2) == MUBLOCKSTORE_ERR_OUT_OF_BOUNDS,
           "read after last block should fail");

    err = store.read(store.getBlockCount()-1, buffer2);
    ASSERT(err == MUBLOCKSTORE_ERR_OK, "read block (err=%d)", err);

    ASSERT(memcmp(buffer1, buffer2, store.getBlockSize()) == 0,
           "read block %lu differs from written block %lu",
           store.getBlockCount(), store.getBlockCount());
}

TEST_MAIN() {
    TEST_START();

    LOG("Using <%s> as a FAT12 test file", MUTEST_FAT12FILE);
    LOG("Using <%s> as a FAT16 test file", MUTEST_FAT16FILE);
    LOG("Using <%s> as a FAT32 test file", MUTEST_FAT32FILE);

    RUN_TEST(create);
    RUN_TEST(seek);
    RUN_TEST(read);
    RUN_TEST(write);

    TEST_END();
}
