#pragma once

// @TODO: Have a mutex locked fallback if we run out of space.

template <typename T, s64 Size>
struct AtomicDoubleBuffer
{
	struct Buffer
	{
		s64 readyCount = 0;
		T data[Size];
	} buffers[2];
	T *writeHead = &buffers[0].data[0];
	Buffer *writeBuffer = &buffers[0];
	Buffer *readBuffer = &buffers[1];
	s64 readBufferElementCount = 0;
};

template <typename T, s64 Size>
void WriteToAtomicDoubleBuffer(AtomicDoubleBuffer<T, Size> *b, const T &newElement)
{
	T *writePointer;
	do
	{
		writePointer = b->writeHead;
		AtomicCompareAndSwapPointer((void *volatile *)&b->writeHead, writePointer, writePointer + 1);
	} while (b->writeHead != writePointer + 1);
	*writePointer = newElement;

	auto bufferIndex = (writePointer < b->buffers[1].data) ? 0 : 1;
	Assert(writePointer - b->buffers[bufferIndex].data < Size);
	b->buffers[bufferIndex].readyCount += 1;
}

template <typename T, s64 Size>
void SwitchAtomicDoubleBuffer(AtomicDoubleBuffer<T, Size> *b)
{
	if (b->writeBuffer == &b->buffers[0])
	{
		b->writeBuffer = &b->buffers[1];
		b->readBuffer = &b->buffers[0];
	}
	else
	{
		b->writeBuffer = &b->buffers[0];
		b->readBuffer = &b->buffers[1];
	}
	auto *oldWriteHead = b->writeHead;
	b->writeBuffer->readyCount = 0;
	b->writeHead = b->writeBuffer->data;
	b->readBufferElementCount = oldWriteHead - b->readBuffer->data;
	while (b->readBufferElementCount != b->readBuffer->readyCount) {} // Busy wait until the final writes finish.
}
