/**
 * \file
 * \brief     Tests for FileStore.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#include "test.hh"
#include "store.hh"

#include <filestore.hh>

TEST_MAIN() {
    TEST_START();

    TEST_STORE_WITH(FileStore(MUTEST_FAT12FILE), create);
    TEST_STORE_WITH(FileStore(MUTEST_FAT12FILE), seek  );
    TEST_STORE_WITH(FileStore(MUTEST_FAT12FILE), read  );
    TEST_STORE_WITH(FileStore(MUTEST_FAT12FILE), write );
    TEST_STORE_WITH(FileStore(MUTEST_FAT12FILE, false), write_ro);

    TEST_END();
}
