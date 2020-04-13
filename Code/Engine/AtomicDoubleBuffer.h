#pragma once

template <typename T, size_t Size>
struct AtomicDoubleBuffer
{
	struct Buffer
	{
		s32 readyCount = 0;
		T data[Size];
	} buffers[2];
	T *writeHead = buffers[0].commandBuffers;
	Buffer *writeBuffer = &buffers[0];
	Buffer *readBuffer = &buffers[1];
	s32 readBufferElementCount = 0;
};

template <typename T, size_t Size>
void Write(AtomicDoubleBuffer<T> *b, const T &&newElement)
{
	T *writePointer;
	do
	{
		writePointer = b->writeHead;
		AtomicCompareAndSwap((void *volatile *)&b->writeHead, writePointer, writePointer + 1);
	} while (b->writeHead != writePointer + 1);
	*writePointer = newElement;

	auto elementIndex = writePointer - b->buffers[bufferIndex].data;
	Assert(elementIndex < Size);
	auto bufferIndex = (writePointer < b->buffers[1].commandBuffers) ? 0 : 1;
	b->buffers[bufferIndex].readyCount += 1;
}

template <typename T, size_t Size>
void SwitchBuffers(AtomicDoubleBuffer<T> *b)
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
	b->writeHead = b->writeBuffer->commandBuffers;
	b->readBufferElementCount = oldWriteHead - b->readBuffer->commandBuffers;
	while (b->readBufferElementCount != b->readBuffer->readyCount) {}
}
