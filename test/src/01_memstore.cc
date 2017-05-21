/**
 * \file
 * \brief     Tests for MemStore.
 * \author    Chris Smeele
 * \copyright Copyright (c) 2016, Chris Smeele
 * \license   Boost, see LICENSE
 */
#include "test.hh"
#include "store.hh"

#include <array>
#include <memstore.hh>

TEST_MAIN() {
    TEST_START();

    auto image = std::array<uint8_t, 128 * 512>();

    image[510] = 0x55; // Insert boot sector signature.
    image[511] = 0xaa;

    TEST_STORE_WITH(MemStore(&image, image.size()), create);
    TEST_STORE_WITH(MemStore(&image, image.size()), seek  );
    TEST_STORE_WITH(MemStore(&image, image.size()), read  );
    TEST_STORE_WITH(MemStore(&image, image.size()), write );

    auto const image_ro = image;

    TEST_STORE_WITH(MemStore(&image_ro, image_ro.size()), write_ro);

    TEST_END();
}
