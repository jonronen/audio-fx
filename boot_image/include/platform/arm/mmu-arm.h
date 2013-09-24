/*  This file MUST be included in your system-target.h file if you want arm
 *  cache coherence functions to be called (I.E. during codec load, etc).   
 */

#ifndef MMU_ARM_H
#define MMU_ARM_H

#define CACHE_ALL   0x0C
#define CACHE_NONE  0
#define BUFFERED    0x04

void memory_init(void);
void ttb_init(void);
void enable_mmu(void);
void map_section(unsigned int pa, unsigned int va, int mb, int flags);

/* Note for the function names
 *
 * ARM refers to the cache coherency functions as (in the CPU manuals):
 *  clean (write-back)
 *  clean and invalidate (write-back and removing the line from cache)
 *  invalidate (removing from cache without write-back)
 *
 * The deprecated functions below don't follow the above (which is why
 * they're deprecated).
 *
 * This names have been proven to cause confusion, therefore we use:
 *  commit
 *  commit and discard
 *  discard
 */

/* Commits entire DCache */
void commit_dcache(void);

/* Commit and discard entire DCache, will do writeback */
void commit_discard_dcache(void);

/* Write DCache back to RAM for the given range and remove cache lines
 * from DCache afterwards */
void commit_discard_dcache_range(const void *base, unsigned int size);

/* Write DCache back to RAM for the given range */
void commit_dcache_range(const void *base, unsigned int size);

/*
 * Remove cache lines for the given range from DCache
 * will *NOT* do write back except for buffer edges not on a line boundary
 */
void discard_dcache_range(const void *base, unsigned int size);

/* Discards the entire ICache, and commit+discards the entire DCache */
void commit_discard_idcache(void);

#endif /* MMU_ARM_H */
