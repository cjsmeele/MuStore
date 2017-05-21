/**
 * \file
 * \brief     Tests for Store providers.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   Boost, see LICENSE
 *
 * Note: These tests assume the provided media contain a 0xaa55 boot
 * sector signature.
 */
#pragma once

#include "test.hh"
#include <store.hh>

using namespace MuStore;

Store *store;

#define TEST_STORE_WITH(x, test)              \
    {                                         \
        auto _store = x;                      \
        store = &_store;                      \
        RUN_TEST(test);                       \
    }

TEST(create) {
    ASSERT(store, "store was not created");
    ASSERT(store->getPos() == 0, "initial pos must be zero (pos=%lu)", store->getPos());

    StoreError err = store->seek(0);
    ASSERT(err == STORE_ERR_OK, "seek to start after creation (err=%d)", err);
}

TEST(seek) {
    ASSERT(store, "store was not created");
    StoreError err;

    LOG("Reported block size: %lu, block count: %lu",
        store->getBlockSize(), store->getBlockCount());

    err = store->seek(0);
    ASSERT(err == STORE_ERR_OK, "seek to start (err=%d)", err);
    size_t curPos = store->getPos();
    ASSERT(curPos == 0, "position 0 after seek to start (pos=%lu)", curPos);

    size_t endPos = store->getBlockCount() - 1;

    err = store->seek(endPos);
    ASSERT(err == STORE_ERR_OK, "seek to end (err=%d)", err);
    curPos = store->getPos();
    ASSERT(curPos == endPos, "position %lu after seek to end (pos=%lu)", endPos, curPos);

    err = store->seek(endPos + 1);
    ASSERT(err == STORE_ERR_OUT_OF_BOUNDS, "seek past end should fail");
}

TEST(read) {
    ASSERT(store, "store was not created");
    StoreError err;

    uint8_t buffer[4096];
    ASSERT(store->getBlockSize() >= 512,  "block size too small");
    ASSERT(store->getBlockSize() <= 4096, "can't test, block size too large");

    err = store->read(buffer);

    ASSERT(err == STORE_ERR_OK, "read block (err=%d)", err);

    uint16_t sig = ((uint16_t)(buffer[510]) << 8) | buffer[511];
    ASSERT(sig == 0x55aa, "boot sector sig mismatch on read (%#04x != %#04x)", sig, 0x55aa);

    ASSERT(store->getPos() == 1, "pos should be 1 after reading first block (pos=%lu)", store->getPos());
}

TEST(write) {
    ASSERT(store, "store was not created");
    StoreError err;

    ASSERT(store->getBlockSize() >=  512, "block size too small");
    ASSERT(store->getBlockSize() <= 4096, "can't test, block size too large");

    uint8_t buffer1[4096];
    uint8_t buffer2[4096] = { };

    for (size_t i = 0; i < store->getBlockSize(); i++)
        buffer1[i] = rand();

    err = store->write(store->getBlockCount()-1, buffer1);

    ASSERT(err == STORE_ERR_OK, "write block (err=%d)", err);
    ASSERT(store->getPos() == store->getBlockCount(),
           "pos should be %lu after writing last block (pos=%lu)",
           store->getBlockCount(), store->getPos());

    ASSERT(store->read(buffer2) == STORE_ERR_OUT_OF_BOUNDS,
           "read after last block should fail");

    err = store->read(store->getBlockCount()-1, buffer2);
    ASSERT(err == STORE_ERR_OK, "read block (err=%d)", err);

    ASSERT(memcmp(buffer1, buffer2, store->getBlockSize()) == 0,
           "read block %lu differs from written block %lu",
           store->getBlockCount(), store->getBlockCount());
}

TEST(write_ro) {
    ASSERT(store, "store was not created");
    StoreError err;

    ASSERT(store->getBlockSize() >=  512, "block size too small");
    ASSERT(store->getBlockSize() <= 4096, "can't test, block size too large");

    uint8_t buffer[4096] = { };

    err = store->write(store->getBlockCount()-1, buffer);

    ASSERT(err == STORE_ERR_NOT_WRITABLE, "write to read-only medium should fail (err=%d)", err);
    ASSERT(store->getPos() == store->getBlockCount()-1,
           "pos should be %lu (unchanged) after failed write to read-only medium (pos=%lu)",
           store->getBlockCount()-1, store->getPos());
}
