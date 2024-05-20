#ifndef CE_MEMORY_H
#define CE_MEMORY_H

void *CE_MemReserve(CE_u64 length);
int CE_MemRelease(void *addr, CE_u64 length);
int CE_MemCommit(void *addr, CE_u64 length);
int CE_MemDecommit(void *addr, CE_u64 length);

#endif
