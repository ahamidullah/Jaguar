#pragma once

template <typename T, size_t Size>
struct AtomicRingBuffer {
	T Elements[Size] = {};
	volatile s32 ReadIndex = 0;
	volatile s32 WriteIndex = 0;
	bool Ready[Size] = {};
};

template <typename T, size_t Size>
void Write(AtomicRingBuffer<T, Size> *Container, T NewElement) {
	s32 CurrentWriteIndex = 0;
	do {
		// @TODO: Some way to detect overwriting a job that isn't done yet (at least in debug mode).
		CurrentWriteIndex = Container->WriteIndex;
	} while(PlatformCompareAndSwapS32(&Container->WriteIndex, CurrentWriteIndex, (CurrentWriteIndex + 1) % Size) != CurrentWriteIndex);
	Container->Elements[CurrentWriteIndex] = NewElement;
	Container->Ready[CurrentWriteIndex] = true;
}

template <typename T, size_t Size>
bool Read(AtomicRingBuffer<T, Size> *Container, T *Result) {
	auto Found = false;
	while (Container->ReadIndex != Container->WriteIndex) {
		// We need to do this a bit carefully to make sure we stop trying to get read an element if the ring buffer is emptied.
		auto ReadIndex = Container->ReadIndex;
		if (ReadIndex != Container->WriteIndex) {
			auto CurrentReadIndex = PlatformCompareAndSwapS32(&Container->ReadIndex, ReadIndex, (ReadIndex + 1) % Size); \
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
