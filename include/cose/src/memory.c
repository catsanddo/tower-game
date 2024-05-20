#include "memory.h"

#ifdef __linux__
#include <sys/mman.h>

void *CE_MemReserve(CE_u64 length)
{
	void *addr = mmap(NULL, length, PROT_NONE, MAP_PRIVATE | MAP_ANONYMOUS,
		-1, 0);

	return addr == MAP_FAILED ? NULL : addr;
}

int CE_MemRelease(void *addr, CE_u64 length)
{
	return munmap(addr, length);
}

int CE_MemCommit(void *addr, CE_u64 length)
{
	return mprotect(addr, length, PROT_READ | PROT_WRITE);
}

int CE_MemDecommit(void *addr, CE_u64 length)
{
	int result = 0;
	result |= mprotect(addr, length, PROT_NONE);
	result |= madvise(addr, length, MADV_DONTNEED);
	return result;
}
#endif

#ifdef _WIN32
#include <Windows.h>

void *CE_MemReserve(CE_u64 length)
{
	return VirtualAlloc(NULL, length, MEM_RESERVE, PAGE_READWRITE);
}

int CE_MemRelease(void *addr, CE_u64 length)
{
	// Negate the return value for 0 on success as with unix conventions
	return !VirtualFree(addr, 0, MEM_RELEASE);
}

int CE_MemCommit(void *addr, CE_u64 length)
{
	return !VirtualAlloc(addr, length, MEM_COMMIT, PAGE_READWRITE);
}

int CE_MemDecommit(void *addr, CE_u64 length)
{
	return !VirtualFree(addr, length, MEM_DECOMMIT);
}
#endif
