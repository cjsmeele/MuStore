
/**
 * \file
 * \brief     Tests for MuScaleBlockStore.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#include "test.hh"
#include "block.hh"

#include "mufileblockstore.hh"
#include "muscaleblockstore.hh"

TEST_MAIN() {
    TEST_START();

    auto fileStore  = MuFileBlockStore(MUTEST_FAT12FILE);
    TEST_BLOCKSTORE_WITH(MuScaleBlockStore(&fileStore, 4096), create);
    TEST_BLOCKSTORE_WITH(MuScaleBlockStore(&fileStore, 4096), seek  );
    TEST_BLOCKSTORE_WITH(MuScaleBlockStore(&fileStore, 4096), read  );
    TEST_BLOCKSTORE_WITH(MuScaleBlockStore(&fileStore, 4096), write );

    auto roFileStore  = MuFileBlockStore(MUTEST_FAT12FILE, false);
    TEST_BLOCKSTORE_WITH(MuScaleBlockStore(&roFileStore, 4096), write_ro);

    TEST_END();
}
