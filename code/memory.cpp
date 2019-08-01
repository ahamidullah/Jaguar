#include <sys/mman.h>
#include <unistd.h>

size_t platform_page_size;

#define MAP_ANONYMOUS 0x20

void *acquire_platform_memory(size_t size) {
	void *result = mmap(0, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (result == (void *)-1) {
		_abort("Failed to get memory from platform: %s", get_platform_error());
	}

	return result;
}

void release_platform_memory(void *memory, size_t size) {
	s32 return_code = munmap(memory, size);
	if (return_code == -1) {
		_abort("Failed to free memory: %s", get_platform_error());
	}
}

#if 0
#define free_array(a)         free_memory(get_array_header(a), sizeof(*(a)))
#define array_push(a,v)       (maybe_grow_array(a,1), (a)[array_count(a)++] = (v))
#define array_add(a,n)        (maybe_grow_array(a,n), array_count(a)+=(n), &(a)[array_count(a)-(n)])
#define array_last(a)         ((a)[array_count(a)-1])
#define clear_array(a)        (array_count(a)=0)

#define get_array_header(a)   ((s32 *)(a) - 2)
#define sizeof_array_header   (2 * sizeof(u32))
#define array_capacity(a)     get_array_header(a)[0]
#define array_count(a)        get_array_header(a)[1]

#define does_array_need_to_grow(a,n)  ((a)==0 || array_count(a)+(n) >= array_capacity(a))
#define maybe_grow_array(a,n)         (does_array_need_to_grow(a,(n)) ? grow_array(a,n) : 0)
#define grow_array(a,n)               (*((void **)&(a)) = grow_array_actual((a), (n), sizeof(*(a))))

/*
void *allocate_static_array(size_t num_bytes) {
	// Although we don't currently use the header in the case of static arrays,
	char *array = allocate_memory(num_bytes + sizeof_array_header);
	return array + sizeof_array_header;
}
*/

#define allocate_array(t, n) (t *)allocate_array_actual(sizeof(t), n)

void *allocate_array_actual(size_t element_size_in_bytes, size_t count) {
	size_t num_bytes = (count * element_size_in_bytes) + sizeof_array_header;
	if (num_bytes < platform_page_size)
		num_bytes = platform_page_size;

	char *array = (char *)allocate_memory(num_bytes);

	((s32 *)array)[0] = count;
	((s32 *)array)[1] = 0;

	return array + sizeof_array_header;
}

void *grow_array_actual(void *array, size_t num_new_elements, size_t element_size_in_bytes) {
	size_t new_count = array_count(array) + num_new_elements;
	size_t new_capacity = new_count * 2;

	char *new_array = (char *)allocate_memory(new_capacity * element_size_in_bytes + sizeof_array_header);
	size_t old_size = array_count(array) * element_size_in_bytes + sizeof_array_header;
	memcpy(new_array, get_array_header(array), old_size);
	free_memory(get_array_header(array), array_capacity(array) * element_size_in_bytes);

	((s32 *)new_array)[0] = new_capacity;
	((s32 *)new_array)[1] = new_count;

	return new_array;
}
#endif

/*
#define array_count(a) array_count_actual(a, sizeof(*(a)))
size_t array_count(void *array, size_t element_size_in_bytes) {
	Array_Header *header = (Array_Header *)((char *)array - sizeof(Array_Header));
	return header->count;
}

#define clear_array(a) ((a) = clear_array_actual((a)))

void *clear_array_actual(void *array) {
	Array_Header *header = (Array_Header *)((char *)array - sizeof(Array_Header));
	header->count = 0;
}

void free_array(void *array) {
	Array_Header *header = (Array_Header *)((char *)array - sizeof(Array_Header));
	free_memory(header, header->count * header->element_size_in_bytes);
}

#define array_push(a, n) ((a) = array_push_actual((a), (n)))

void *array_push_actual(void *array, size_t num_elements) {
	Array_Header *header = (Array_Header *)((char *)array - sizeof(Array_Header));
	size_t new_count = header->count + num_elements;
	if (new_count > header->capacity) {
		size_t new_capacity = new_count * 2;
		char *new_array = allocate_memory(new_capacity * header->element_size_in_bytes + sizeof(Array_Header));
		size_t old_size = header->count * header->element_size_in_bytes + sizeof(Array_Header);
		memcpy(new_array, header, old_size);
		free_memory(header, old_size);

		header = (Array_Header *)new_array;
		header->count = new_count;
		header->capacity = new_capacity;
		array = (char *)new_array + sizeof(Array_Header);
	}
	return array;
}
*/

#define BLOCK_DATA_SIZE ((platform_page_size * 256) - sizeof(Block_Header))
#define BLOCK_DATA_PLUS_HEADER_SIZE (BLOCK_DATA_SIZE + sizeof(Block_Header))
#define CHUNK_DATA_SIZE (BLOCK_DATA_PLUS_HEADER_SIZE * 1024)
#define CHUNK_DATA_PLUS_HEADER_SIZE (CHUNK_DATA_SIZE + sizeof(Chunk_Header))

Chunk_Header *memory_chunks;
Chunk_Header *active_memory_chunk;
Block_Header *memory_block_free_head;

void *get_platform_memory(size_t len) {
	void *memory = mmap(0, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (memory == (void *)-1)
		_abort("failed to get memory from platform.");
	return memory;
}

void free_platform_memory(void *m, size_t len) {
	s32 ret = munmap(m, len);
	if (ret == -1)
		_abort("failed to free memory.");
}

size_t get_platform_page_size() {
	return sysconf(_SC_PAGESIZE);
}

Chunk_Header *make_memory_chunk() {
	Chunk_Header *chunk = (Chunk_Header *)get_platform_memory(CHUNK_DATA_PLUS_HEADER_SIZE);
	chunk->base_block = (u8 *)chunk;
	chunk->block_frontier = (u8 *)chunk->base_block + sizeof(Chunk_Header);
	chunk->next_chunk = NULL;
	return chunk;
}

u8 *get_block_start(Block_Header *block) {
	return ((u8 *)block + sizeof(Block_Header));
}

Block_Header *get_block_header(u8 *start) {
	return (Block_Header *)(start - sizeof(Block_Header));
}

Block_Header *create_memory_block() {
	Block_Header *block;
	if (memory_block_free_head) {
		block = memory_block_free_head;
		memory_block_free_head = memory_block_free_head->next_block;
	} else {
		if (((active_memory_chunk->block_frontier + BLOCK_DATA_PLUS_HEADER_SIZE) - active_memory_chunk->base_block) > CHUNK_DATA_SIZE) {
			active_memory_chunk->next_chunk = make_memory_chunk();
			active_memory_chunk = active_memory_chunk->next_chunk;
		}
		block = (Block_Header *)active_memory_chunk->block_frontier;
		active_memory_chunk->block_frontier += BLOCK_DATA_PLUS_HEADER_SIZE;
	}
	block->byte_capacity = BLOCK_DATA_SIZE;
	block->bytes_used = 0;
	block->next_block = NULL;
	block->previous_block = NULL;
	return block;
}

Memory_Arena make_memory_arena() {
	Memory_Arena arena;
	arena.entry_free_head = NULL;
	arena.last_entry = NULL;
	arena.base_block = create_memory_block();
	arena.active_block = arena.base_block;
	return arena;
}

void clear_memory_arena(Memory_Arena *arena) {
	arena->active_block = arena->base_block;
	arena->active_block->bytes_used = 0;
}

void free_memory_block(Block_Header *block) {
	block->next_block = memory_block_free_head;
	memory_block_free_head = block;
}

void free_memory_arena(Memory_Arena *arena) {
	for (Block_Header *block = arena->base_block, *next = NULL; block; block = next) {
		next = block->next_block;
		free_memory_block(block);
	}
	//ma->base = ma->active_block = NULL;
}

void initialize_memory(Game_State *game_state) {
	platform_page_size = get_platform_page_size();
	memory_chunks = make_memory_chunk();
	active_memory_chunk = memory_chunks;
	memory_block_free_head = NULL;

	game_state->frame_arena = make_memory_arena();
	//game_state->assets.arena = make_memory_arena();
	//game_state->permanant_arena = make_memory_arena();
}

#if 0
// Complicated, becuase we might have to swap bytes across blocks or chunks that are not adjacent.
// Some edge cases: start and end the same
//                  start pointer and end pointer both change blocks as the reverse ends
//                  end pointer is the first byte of a new block
//                  start and end in different chunks
// Do we even use this anymore?
void
mem_reverse(Arena_Address *start, Arena_Address *end, Memory_Arena *ma)
{
	assert(start->byte_addr >= get_block_start(start->block_footer) && start->byte_addr < (char *)start->block_footer);
	assert(end->byte_addr >= get_block_start(end->block_footer) && end->byte_addr < (char *)end->block_footer);

	// Since mem_end returns one byte past the last byte written, we want to move our end pointer back one so we're pointing to the last byte written.
	if (--end->byte_addr < get_block_start(end->block_footer)) { // Have we gone past the start of the block?
		end->block_footer = end->block_footer->prev;
		assert(end->block_footer);
		end->byte_addr = (char *)end->block_footer - 1;
	}

	bool swapped_places = false;
	while (start->byte_addr != end->byte_addr && !swapped_places) {
		char tmp = *start->byte_addr;
		*start->byte_addr = *end->byte_addr;
		*end->byte_addr = tmp;

		char *prev_end = end->byte_addr;
		if (++start->byte_addr >= (char *)start->block_footer) { // Have we gone off the end of the block?
			start->block_footer = start->block_footer->next;
			assert(start->block_footer);
			start->byte_addr = get_block_start(start->block_footer);
		}
		if (--end->byte_addr < get_block_start(end->block_footer)) { // Have we gone past the start of the block?
			end->block_footer = end->block_footer->prev;
			assert(end->block_footer);
			end->byte_addr = (char *)end->block_footer - 1;
		}
		swapped_places = start->byte_addr == prev_end;
	}
}

char *get_entry_data(Entry_Header *e) {
	return (char *)e + sizeof(Entry_Header);
}

Entry_Header *get_entry_header(void *p) {
	return (Entry_Header *)((char *)p - sizeof(Entry_Header));
}

void *start_of_memory_arena(Memory_Arena *arena) {
	// TODO: It's actually not. Need to make it so that we don't get a block on startup incase the first allocation is bigger than one block.
	if (arena->last_entry) // Do we have any entries? If we do, it's guarenteed that the first one is right at the start of the base block.
		return get_entry_data((Entry_Header *)get_block_start(arena->base));
	return NULL;
}

void *next_memory_arena_entry(void *p) {
	return get_entry_header(p)->next;
}
#endif

#if 0
bool
mem_has_elems(Memory_Arena *ma)
{
	return ma->last_entry;
}
#endif

void add_memory_arena_block(Memory_Arena *arena) {
	Block_Header *new_block = create_memory_block();
	new_block->previous_block = arena->active_block;
	arena->active_block->next_block = new_block;
	arena->active_block = new_block;
}

/*
#define mem_alloc(type, arena) (type *)mem_push(sizeof(type), arena)
#define mem_alloc_array(type, count, arena) (type *)mem_push_contiguous(sizeof(type)*count, arena)
*/

s32 round_up(f32 f) {
	return (f > 0.0f) ? (s32)(f + 1.0f) : (s32)(f - 1.0f);
}

// @TODO: Align allocated memory?
#define allocate_array(arena, type, count) (type *)memory_arena_allocate(arena, sizeof(type) * (count))
#define allocate_struct(arena, type) (type *)memory_arena_allocate(arena, sizeof(type))

void *memory_arena_allocate(Memory_Arena *arena, size_t size) {
	void *result = (char *)arena->active_block + sizeof(Block_Header) + arena->active_block->bytes_used;
	arena->active_block->bytes_used += size;
	assert(arena->active_block->bytes_used < BLOCK_DATA_SIZE);
	return result;
}

/*
// TODO: combine the block group logic here with mem_make_block and just call that.
void *memory_arena_allocate(Memory_Arena *arena, size_t size) {
	assert(size <= CHUNK_DATA_SIZE);
	size_t num_blocks_needed = round_up((f32)(size + sizeof(Block_Header)) / BLOCK_DATA_PLUS_HEADER_SIZE);

	// We could try to find our needed blocks among the free blocks, but for now we just take what we need from the block frontier of the chunk.
	// Is there enough space in our current chunk for the contiguous memory requested?
	if (active_memory_chunk->block_frontier + (BLOCK_DATA_PLUS_HEADER_SIZE * num_blocks_needed) - active_memory_chunk->base_block > CHUNK_DATA_SIZE) {
		// Add all of the unused blocks in the current chunk to the free list.
		while (active_memory_chunk->block_frontier + BLOCK_DATA_PLUS_HEADER_SIZE - active_memory_chunk->base_block > CHUNK_DATA_SIZE) {
			free_memory_block(get_block_header(active_memory_chunk->block_frontier));
			active_memory_chunk->block_frontier += BLOCK_DATA_PLUS_HEADER_SIZE;
		}
		active_memory_chunk->next_chunk = make_memory_chunk();
		active_memory_chunk = active_memory_chunk->next_chunk;
	}
	// Move us forward to the last block, where we will keep the footer for the enitre block group.
	Block_Header *block = get_block_header(active_memory_chunk->block_frontier + ((num_blocks_needed - 1) * BLOCK_DATA_PLUS_HEADER_SIZE));
	block->byte_capacity = num_blocks_needed * BLOCK_DATA_PLUS_HEADER_SIZE - sizeof(Block_Header);
	block->bytes_used = size;
	block->previous_block = arena->active_block;
	block->next_block = NULL;
	arena->active_block->next_block = block;
	arena->active_block = block;
	active_memory_chunk->block_frontier += BLOCK_DATA_PLUS_HEADER_SIZE * num_blocks_needed;
	return get_block_start(block);
}
*/

// TODO: Keep track of the largest free entry available to speed up allocation?
void *memory_arena_push(size_t size, Memory_Arena *ma) {
	return NULL;
#if 0
	size_t size_with_header = size + sizeof(Entry_Header);
	assert(size_with_header <= BLOCK_DATA_SIZE);
	/*
	for (Free_Entry *f = ma->entry_free_head, *prev = NULL; f; prev = f, f = f->next) {
		// TODO: Re-add the remaining free entry space to the free entry list.
		if (size <= f->size) {
			// THIS IS BUSTED.
			if (prev)
				prev->next = f->next;
			return (char *)f + sizeof(Entry_Header);
		}
	}
	*/
	Entry_Header *new_entry_header;
	if (size_with_header <= (ma->active_block->capacity - ma->active_block->nbytes_used))
		new_entry_header = (Entry_Header *)(get_block_start(ma->active_block) + ma->active_block->nbytes_used);
	else { // Need a new block.
		add_memory_arena_block(ma);
		new_entry_header = (Entry_Header *)get_block_start(ma->active_block);
	}

	char *new_entry = get_entry_data(new_entry_header);
	new_entry_header->size = size;
	new_entry_header->next = NULL;
	new_entry_header->prev = ma->last_entry;
	if (ma->last_entry) {
		Entry_Header *last_entry_header = get_entry_header(ma->last_entry);
		last_entry_header->next = new_entry;
	}

	ma->last_entry = new_entry;
	ma->active_block->nbytes_used += size_with_header;
	// If we fill up the block exactly, we get a new one right away.
	if (ma->active_block->nbytes_used == ma->active_block->capacity)
		add_memory_arena_block(ma);
	return new_entry;
#endif
}

#if 0
void free_memory_arena(Memory_Arena *ma, void *p) {
	assert(p >= (void *)(active_memory_chunk->base + sizeof(Entry_Header)) && p < (void *)active_memory_chunk);
	// TODO: Merge adjacent free entries.
	Free_Entry *f = (Free_Entry *)get_entry_header(p);
	f->next = ma->entry_free_head;
	ma->entry_free_head = f;
}
#endif
