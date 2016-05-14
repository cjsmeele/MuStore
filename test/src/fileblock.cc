/**
 * \file
 * \brief     Tests for MuFileBlockStore.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#include "test.hh"
#include "mufileblockstore.hh"

#include <ctime>
#include "block.hh"

TEST_MAIN() {
    TEST_START();

    TEST_BLOCKSTORE_WITH(MuFileBlockStore(MUTEST_FAT12FILE), create);
    TEST_BLOCKSTORE_WITH(MuFileBlockStore(MUTEST_FAT12FILE), seek  );
    TEST_BLOCKSTORE_WITH(MuFileBlockStore(MUTEST_FAT12FILE), read  );
    TEST_BLOCKSTORE_WITH(MuFileBlockStore(MUTEST_FAT12FILE), write );
    TEST_BLOCKSTORE_WITH(MuFileBlockStore(MUTEST_FAT12FILE, false), write_ro);

    TEST_END();
}
