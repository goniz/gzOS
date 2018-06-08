#include <gtest/gtest.h>
#include <lib/network/nbuf.h>

TEST(nbuf, alloc_1024_returns_non_null)
{
    auto* nbuf = nbuf_alloc(1024);
    ASSERT_NE(nullptr, nbuf);

    nbuf_free(nbuf);
}

TEST(nbuf, alloc_zero_returns_null)
{
    auto* nbuf = nbuf_alloc(0);
    ASSERT_EQ(nullptr, nbuf);
}

TEST(nbuf, alloc_1024_returns_nbuf_is_valid)
{
    auto* nbuf = nbuf_alloc(1024);

    ASSERT_TRUE(nbuf_is_valid(nbuf));

    nbuf_free(nbuf);
}

TEST(nbuf, alloc_1024_returns_nbuf_with_1024_capacity)
{
    auto* nbuf = nbuf_alloc(1024);

    ASSERT_EQ(1024, nbuf_capacity(nbuf));

    nbuf_free(nbuf);
}

TEST(nbuf, alloc_1024_returns_nbuf_with_1024_size)
{
    auto* nbuf = nbuf_alloc(1024);

    ASSERT_EQ(1024, nbuf_size(nbuf));

    nbuf_free(nbuf);
}

TEST(nbuf, alloc_clone_returns_non_null)
{
    auto* nbuf = nbuf_alloc(1024);
    auto* clone = nbuf_clone(nbuf);

    ASSERT_NE(nullptr, clone);

    nbuf_free(nbuf);
    nbuf_free(clone);
}