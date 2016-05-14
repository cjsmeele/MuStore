/**
 * \file
 * \brief     Tests for MuMemBlockStore.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   LGPLv3+, see LICENSE
 */
#include "test.hh"
#include "mumemblockstore.hh"

#include <array>
#include "block.hh"

TEST_MAIN() {
    TEST_START();

    auto image = std::array<uint8_t, 128 * 512>(); // 256K.

    image[510] = 0x55; // Insert boot sector signature.
    image[511] = 0xaa;

    TEST_BLOCKSTORE_WITH(MuMemBlockStore(&image, image.size()), create);
    TEST_BLOCKSTORE_WITH(MuMemBlockStore(&image, image.size()), seek  );
    TEST_BLOCKSTORE_WITH(MuMemBlockStore(&image, image.size()), read  );
    TEST_BLOCKSTORE_WITH(MuMemBlockStore(&image, image.size()), write );

    TEST_END();
}
