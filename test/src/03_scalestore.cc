
/**
 * \file
 * \brief     Tests for ScaleStore.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#include "test.hh"
#include "store.hh"

#include <filestore.hh>
#include <scalestore.hh>

TEST_MAIN() {
    TEST_START();

    auto fileStore = FileStore(MUTEST_FAT12FILE);
    TEST_STORE_WITH(ScaleStore(&fileStore, 4096), create);
    TEST_STORE_WITH(ScaleStore(&fileStore, 4096), seek  );
    TEST_STORE_WITH(ScaleStore(&fileStore, 4096), read  );
    TEST_STORE_WITH(ScaleStore(&fileStore, 4096), write );

    auto roFileStore = FileStore(MUTEST_FAT12FILE, false);
    TEST_STORE_WITH(ScaleStore(&roFileStore, 4096), write_ro);

    TEST_END();
}
