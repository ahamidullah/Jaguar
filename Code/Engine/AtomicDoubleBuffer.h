#pragma once

template <typename T, size_t Size>
struct AtomicDoubleBuffer {
	struct Buffer {
		s32 readyCount = 0;
		T data[Size];
	} buffers[2];
	T *writeHead = &buffers[0];
	writeBuffer = &buffers[0];
	container.write_head = container.buffers[0].command_buffers ; \
	container.read_buffer_element_count = 0; \
	container.read_buffer = &container.buffers[1]; \
	container.write_buffer->ready_count = 0; \
	container.read_buffer->ready_count = 0; \
};

template <typename T, size_t Size>
void AtmoicWriteToDoubleBuffer(AtomicDoubleBuffer<T> *b, const T &&newElement) {
	T *writePointer;
	do {
		writePointer = container.writeHead;
		PlatformCompareAndSwapPointers((void *volatile *)&container.write_head, write_pointer, write_pointer + 1); \
	} while (container.write_head != write_pointer + 1); \
	*write_pointer = command_buffer; \
	s32 buffer_index = (write_pointer < container.buffers[1].command_buffers) ? 0 : 1; \
	s32 element_index = write_pointer - container.buffers[buffer_index].command_buffers; \
	container.buffers[buffer_index].gpu_upload_counters[element_index] = counter; \
	container.buffers[buffer_index].ready_count += 1; \
	/*Console_Print("buffer_index: %d %d\n", buffer_index, element_index);*/ \
}
