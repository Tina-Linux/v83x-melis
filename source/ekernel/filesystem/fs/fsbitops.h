/*
*********************************************************************************************************
*                                                    MELIS
*                                    the Easy Portable/Player Develop Kits
*                                                  File System
*
*                                    (c) Copyright 2011-2014, Sunny China
*                                             All Rights Reserved
*
* File    : fsbitops.h
* By      : Sunny
* Version : v1.0
* Date    : 2011-1-15
* Descript: normal bit operations routine, code is extracted from linux.
* Update  : date                auther      ver     notes
*           2011-3-15 15:21:11  Sunny       1.0     Create this file.
*********************************************************************************************************
*/

#ifndef  __FSBITOPS_H__
#define  __FSBITOPS_H__

#define BIT(nr)             (1UL << (nr))
#define BIT_MASK(nr)        (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)        ((nr) / BITS_PER_LONG)
#define BITS_TO_LONGS(nr)   DIV_ROUND_UP(nr, BITS_PER_LONG)
#define BITS_PER_BYTE       8

/**
 * __test_and_set_bit - Set a bit and return its old value
 * @nr: Bit to set
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.
 * If two examples of this operation race, one can appear to succeed
 * but actually fail.  You must protect multiple accesses with a lock.
 */
static inline int __test_and_set_bit(int nr, volatile unsigned long *addr)
{
    unsigned long mask = BIT_MASK(nr);
    unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
    unsigned long old = *p;

    *p = old | mask;
    return (old & mask) != 0;
}

/**
 * __test_and_clear_bit - Clear a bit and return its old value
 * @nr: Bit to clear
 * @addr: Address to count from
 *
 * This operation is non-atomic and can be reordered.
 * If two examples of this operation race, one can appear to succeed
 * but actually fail.  You must protect multiple accesses with a lock.
 */
static inline int __test_and_clear_bit(int nr, volatile unsigned long *addr)
{
    unsigned long mask = BIT_MASK(nr);
    unsigned long *p = ((unsigned long *)addr) + BIT_WORD(nr);
    unsigned long old = *p;

    *p = old & ~mask;
    return (old & mask) != 0;
}

static inline int ext2_test_bit(int nr, const void *vaddr)
{
    const unsigned char *p = vaddr;
    return (p[nr >> 3] & (1U << (nr & 7))) != 0;
}

static inline int ext2_find_first_bit(const void *vaddr, unsigned size)
{
    const unsigned long *p = vaddr, *addr = vaddr;
    int res;

    if (!size)
    {
        return 0;
    }

    size = (size >> 5) + ((size & 31) > 0);
    while (*p++ == 0UL)
    {
        if (--size == 0)
        {
            return (p - addr) << 5;
        }
    }

    --p;
    for (res = 0; res < 32; res++)
        if (ext2_test_bit(res, p))
        {
            break;
        }
    return (p - addr) * 32 + res;
}

static inline int ext2_find_next_bit(const void *vaddr, unsigned size,
                                     unsigned offset)
{
    const unsigned long *addr = vaddr;
    const unsigned long *p = addr + (offset >> 5);
    int bit = offset & 31UL, res;

    if (offset >= size)
    {
        return size;
    }

    if (bit)
    {
        /* Look for one in first longword */
        for (res = bit; res < 32; res++)
            if (ext2_test_bit(res, p))
            {
                return (p - addr) * 32 + res;
            }
        p++;
    }
    /* No set bit yet, search remaining full bytes for a set bit */
    res = ext2_find_first_bit(p, size - 32 * (p - addr));
    return (p - addr) * 32 + res;
}

/**
 * __ffs - find first bit in word.
 * @word: The word to search
 *
 * Undefined if no bit exists, so code should check against 0 first.
 */
static inline unsigned long __ffs(unsigned long word)
{
    int num = 0;

#if BITS_PER_LONG == 64
    if ((word & 0xffffffff) == 0)
    {
        num += 32;
        word >>= 32;
    }
#endif
    if ((word & 0xffff) == 0)
    {
        num += 16;
        word >>= 16;
    }
    if ((word & 0xff) == 0)
    {
        num += 8;
        word >>= 8;
    }
    if ((word & 0xf) == 0)
    {
        num += 4;
        word >>= 4;
    }
    if ((word & 0x3) == 0)
    {
        num += 2;
        word >>= 2;
    }
    if ((word & 0x1) == 0)
    {
        num += 1;
    }
    return num;
}


/*
 * ffz = Find First Zero in word. Undefined if no zero exists,
 * so code should check against ~0UL first..
 */
static inline  unsigned long ffz(unsigned long word)
{
    return __ffs(~word);
}

#define ext2_set_bit(nr, addr)           __test_and_set_bit((nr) ^ 24, (unsigned long *)(addr))
#define ext2_set_bit_atomic(lock, nr, addr) test_and_set_bit((nr) ^ 24, (unsigned long *)(addr))
#define ext2_clear_bit(nr, addr)    __test_and_clear_bit((nr) ^ 24, (unsigned long *)(addr))
#define ext2_clear_bit_atomic(lock, nr, addr)   test_and_clear_bit((nr) ^ 24, (unsigned long *)(addr))

#endif  /* __FSBITOPS_H__ */

