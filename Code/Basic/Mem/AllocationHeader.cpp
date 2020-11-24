#include "AllocationHeader.h"
#include "Memory.h"

namespace Memory
{

u8 *SetAllocationHeaderAndData(void *mem, s64 size, s64 align)
{
	auto hdr = (u8 *)AlignPointer(mem, alignof(AllocationHeader));
	auto dat = hdr + sizeof(AllocationHeader);
	if ((PointerInt)dat % align == 0)
	{
		// Make room to store number of bytes between the end of the header and the start of the data.  We need this when we go to free the
		// pointer.
		dat += align;
	}
	else
	{
		dat = (u8 *)AlignPointer(dat, align);
	}
	// Right now we store the number of bytes from the start of the data to the start of the header, but we could store from the start of
	// data to the end of the header. That would save us if the size of the AllocationHeader ever got too big...
	auto nBytesFromDataToHeader = dat - hdr;
	if (nBytesFromDataToHeader > U8Max)
	{
		// Move the header up.
		hdr = (u8 *)AlignPointer(dat - sizeof(AllocationHeader) - alignof(AllocationHeader), alignof(AllocationHeader));
		Assert(hdr >= mem);
		Assert(hdr < dat);
		nBytesFromDataToHeader = dat - hdr;
		Assert(nBytesFromDataToHeader < U8Max);
		Assert(nBytesFromDataToHeader >= sizeof(AllocationHeader));
	}
	((AllocationHeader *)hdr)->size = size;
	((AllocationHeader *)hdr)->alignment = align;
	*(dat - 1) = nBytesFromDataToHeader;
	return dat;
}

AllocationHeader *GetAllocationHeader(void *mem)
{
	auto nBytesFromDataToHeader = *((u8 *)mem - 1);
	return (AllocationHeader *)((u8 *)mem - nBytesFromDataToHeader);
}

}
