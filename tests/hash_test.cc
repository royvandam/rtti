#include <gtest/gtest.h>

#include <hash.hh>
#include <string_view>

namespace {
    TEST(Hash, KnownFnv1aVectors) {
        // Independent FNV-1a vectors; do not calculate expectations with FNV1a.
        static_assert(Hash::FNV1a("") == 0x811c9dc5u);
        static_assert(Hash::FNV1a("a") == 0xe40c292cu);
        static_assert(Hash::FNV1a("hello") == 0x4f9f2cabu);
        static_assert(Hash::FNV1a("foobar") == 0xbf9cf968u);

        EXPECT_EQ(Hash::FNV1a(""), 0x811c9dc5u);
        EXPECT_EQ(Hash::FNV1a("a"), 0xe40c292cu);
        EXPECT_EQ(Hash::FNV1a("hello"), 0x4f9f2cabu);
        EXPECT_EQ(Hash::FNV1a("foobar"), 0xbf9cf968u);
    }

    TEST(Hash, EmptyInputPreservesSeed) {
        static_assert(Hash::FNV1a(nullptr, 0) == 0x811c9dc5u);
        static_assert(Hash::FNV1a(nullptr, 0, 42u) == 42u);
        EXPECT_EQ(Hash::FNV1a(std::string_view{}), 0x811c9dc5u);
        EXPECT_EQ(Hash::FNV1a("ignored", 0, 42u), 42u);
    }

    TEST(Hash, ExplicitLengthDoesNotReadPastInput) {
        constexpr char input[] = {'h', 'e', 'l', 'l', 'o', '!'};
        static_assert(Hash::FNV1a(input, 5) == 0x4f9f2cabu);
        EXPECT_EQ(Hash::FNV1a(input, 5), 0x4f9f2cabu);
        EXPECT_EQ(Hash::FNV1a(std::string_view(input, 5)), 0x4f9f2cabu);
    }

    TEST(Hash, EmbeddedNullIsHashedAsAByte) {
        constexpr char input[] = {'a', '\0', 'b'};
        static_assert(Hash::FNV1a(input, 3) == 0x10f3abd2u);
        EXPECT_EQ(Hash::FNV1a(input, 3), 0x10f3abd2u);
        EXPECT_EQ(Hash::FNV1a(std::string_view(input, 3)), 0x10f3abd2u);
    }

    TEST(Hash, SeedContinuesAnExistingHash) {
        static_assert(Hash::FNV1a("bar", 3, 0xa9f37ed7u) == 0xbf9cf968u);
        EXPECT_EQ(Hash::FNV1a("bar", 3, 0xa9f37ed7u), 0xbf9cf968u);
    }

    TEST(Hash, HighBitBytesAreUnsignedOctets) {
        constexpr char input[] = "\xff\x80";
        static_assert(Hash::FNV1a(input, 2) == 0xee1eea4au);
        EXPECT_EQ(Hash::FNV1a(input, 2), 0xee1eea4au);
        EXPECT_EQ(Hash::FNV1a(std::string_view(input, 2)), 0xee1eea4au);
    }
}  // namespace
