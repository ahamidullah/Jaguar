template <typename T, s64 Size>
struct AtomicBuffer
{
	T elements[Size] = {};
	volatile s64 endIndex = 0;
	volatile s64 readyCount = 0;
};

template <typename T, s64 Size>
void WriteToAtomicBuffer(AtomicBuffer<T, Size> *buffer, T newElement)
{
	if (buffer->endIndex >= Size)
	{
		// @TODO: Handle overflow.
		Abort("Atomic buffer overflow.");
	}
	auto writeIndex = AtomicFetchAndAdd(&buffer->endIndex, 1);
	buffer->elements[writeIndex] = newElement;
	AtomicAdd(&buffer->readyCount, 1);
}

template <typename T, s64 Size>
void ClearAtomicBuffer(AtomicBuffer<T, Size> *buffer)
{
	buffer->readyCount = 0;
	buffer->endIndex = 0;
}
