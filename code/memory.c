size_t platform_page_size;

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
	game_state->permanent_arena = make_memory_arena();
}

void add_memory_arena_block(Memory_Arena *arena) {
	Block_Header *new_block = create_memory_block();
	new_block->previous_block = arena->active_block;
	arena->active_block->next_block = new_block;
	arena->active_block = new_block;
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
