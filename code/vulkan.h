struct Vulkan_Memory_Allocation;

typedef struct {
	struct Vulkan_Memory_Allocation *vertices;
	struct Vulkan_Memory_Allocation *indices;
	// @TODO: Something to do with material ids.
} GPU_Mesh;
