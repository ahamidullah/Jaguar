#if 0
#define RANDOM_COLOR_TABLE_LENGTH 1024
V3 random_color_table[RANDOM_COLOR_TABLE_LENGTH];

void Add_Debug_Render_Object(Render_Context *context, void *vertices, u32 vertex_count, size_t sizeof_vertex, u32 *indices, u32 index_count, V4 color, Render_Primitive primitive) {
	Debug_Render_Object *object = &renderGlobals.debug_render_objects[renderGlobals.debug_render_object_count++];
	*object = (Debug_Render_Object){
		.index_count = index_count,
		.color = color,
		.render_primitive = primitive,
	};
	//vulkan_push_debug_vertices(vertices, vertex_count, sizeof_vertex, indices, index_count, &object->vertex_offset, &object->first_index);
	Assert(renderGlobals.debug_render_object_count < MAX_DEBUG_RENDER_OBJECTS);
}

void Draw_Wire_Sphere(Render_Context *context, V3 center, f32 radius, V4 color) {
	const u32 sector_count = 30;
	const u32 stack_count  = 15;
	f32 sector_step = 2.0f * M_PI / (f32)sector_count;
	f32 stack_step = M_PI / (f32)stack_count;
	f32 sector_angle, stack_angle;
	f32 x, y, z, xy;
	Vertex1P vertices[((sector_count + 1) * (stack_count + 1))];
	u32 vertex_count = 0;
	for (u32 i = 0; i <= stack_count; ++i) {
		stack_angle = M_PI / 2 - i * stack_step; // From pi/2 to -pi/2.
		xy = radius * cosf(stack_angle); // r * cos(u)
		z = radius * sinf(stack_angle); // r * sin(u)
		for (u32 j = 0; j <= sector_count; ++j) {
			sector_angle = j * sector_step; // From 0 to 2pi.
			x = xy * cosf(sector_angle); // r * cos(u) * cos(v)
			y = xy * sinf(sector_angle); // r * cos(u) * sin(v)
			vertices[vertex_count++].position = (V3){x, y, z} + center;
		}
	}
	u32 indices[4 * stack_count * sector_count];
	u32 index_count = 0;
	u32 k1, k2;
	for (u32 i = 0; i < stack_count; ++i) {
		k1 = i * (sector_count + 1);
		k2 = k1 + sector_count + 1;
		for (u32 j = 0; j < sector_count; ++j, ++k1, ++k2) {
			indices[index_count++] = k1;
			indices[index_count++] = k2;
			if (i != 0) {
				indices[index_count++] = k1;
				indices[index_count++] = k1 + 1;
			}
		}
	}
	Add_Debug_Render_Object(context, vertices, vertex_count, sizeof(vertices[0]), indices, index_count, color, LINE_PRIMITIVE);
}

V3 random_color() {
	float r = rand() / (f32)RAND_MAX;
	float g = rand() / (f32)RAND_MAX;
	float b = rand() / (f32)RAND_MAX;
	return (V3){r, g, b};
}
#endif
