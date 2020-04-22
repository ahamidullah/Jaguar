#pragma once

template <typename T, s64 Size>
struct AtomicRingBuffer {
	T Elements[Size] = {};
	volatile s64 ReadIndex = 0;
	volatile s64 WriteIndex = 0;
	bool Ready[Size] = {};
};

template <typename T, s64 Size>
void WriteToAtomicRingBuffer(AtomicRingBuffer<T, Size> *Container, T NewElement)
{
	auto CurrentWriteIndex = 0;
	do
	{
		// @TODO: Some way to detect overwriting a job that isn't done yet (at least in debug mode).
		CurrentWriteIndex = Container->WriteIndex;
	} while (AtomicCompareAndSwap64(&Container->WriteIndex, CurrentWriteIndex, (CurrentWriteIndex + 1) % Size) != CurrentWriteIndex);
	Container->Elements[CurrentWriteIndex] = NewElement;
	Container->Ready[CurrentWriteIndex] = true;
}

template <typename T, s64 Size>
bool ReadFromAtomicRingBuffer(AtomicRingBuffer<T, Size> *Container, T *Result) {
	auto Found = false;
	while (Container->ReadIndex != Container->WriteIndex) {
		// We need to do this a bit carefully to make sure we stop trying to get read an element if the ring buffer is emptied.
		auto ReadIndex = Container->ReadIndex;
		if (ReadIndex != Container->WriteIndex) {
			auto CurrentReadIndex = AtomicCompareAndSwap64(&Container->ReadIndex, ReadIndex, (ReadIndex + 1) % Size);
			if (CurrentReadIndex == ReadIndex) {
				while (!Container->Ready[ReadIndex]) {}
				*Result = Container->Elements[ReadIndex];
				Found = true;
				break;
			}
		}
	}
	return Found;
}
