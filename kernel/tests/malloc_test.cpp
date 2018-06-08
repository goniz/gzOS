#include <gtest/gtest.h>
#include <lib/malloc/malloc.h>
#include <lib/mm/vm.h>
#include <lib/mm/physmem.h>

MALLOC_DEFINE(test_mp, "test");

__attribute((constructor))
static void setup() {
    vm_page_t* page = pm_alloc(1);
    assert(page != NULL);

    kmalloc_add_arena(test_mp, (void*)page->vaddr, PAGESIZE);
}

TEST(malloc, malloc_16_returns_non_null)
{
    void* ptr = kmalloc(test_mp, 16, M_ZERO);
    ASSERT_NE(nullptr, ptr);

    kfree(test_mp, ptr);
}

TEST(malloc, malloc_0_returns_null)
{
    void* ptr = kmalloc(test_mp, 0, M_ZERO);
    ASSERT_EQ(nullptr, ptr);
}

TEST(malloc, malloc_too_much_returns_null)
{
    void* ptr = kmalloc(test_mp, PAGESIZE * 2, M_NOWAIT);
    ASSERT_EQ(nullptr, ptr);
}

TEST(malloc, malloc_with_zero_flag_returns_memset_memory)
{
    char* ptr = (char*)kmalloc(test_mp, 16, M_NOWAIT | M_ZERO);
    memset(ptr, 0xAA, 16);
    kfree(test_mp, ptr);

    ptr = (char*)kmalloc(test_mp, 16, M_ZERO);

    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(0, ptr[i]);
    }

    kfree(test_mp, ptr);
}

TEST(malloc, malloc_with_zero_and_nowait_flag_returns_memset_memory)
{
    char* ptr = (char*)kmalloc(test_mp, 16, M_NOWAIT | M_ZERO);
    memset(ptr, 0xAA, 16);
    kfree(test_mp, ptr);

    ptr = (char*)kmalloc(test_mp, 16, M_ZERO | M_NOWAIT);

    for (int i = 0; i < 16; i++) {
        ASSERT_EQ(0, ptr[i]);
    }

    kfree(test_mp, ptr);
}

TEST(malloc, malloc_free_malloc_should_return_the_same_address)
{
    void* ptr = kmalloc(test_mp, 16, M_NOWAIT);
    kfree(test_mp, ptr);

    void* ptr2 = kmalloc(test_mp, 16, M_NOWAIT);
    kfree(test_mp, ptr2);

    ASSERT_EQ(ptr, ptr2);
}