#define MAX_VISIBLE_ENTITY_MESHES MAX_ENTITY_MESHES

Render_Context render_context; // @TODO: Move this into the game_state?

#define RANDOM_COLOR_TABLE_LENGTH 1024
V3 random_color_table[RANDOM_COLOR_TABLE_LENGTH];

void add_debug_render_object(void *vertices, u32 vertex_count, size_t sizeof_vertex, u32 *indices, u32 index_count, V4 color, Render_Primitive primitive) {
	Debug_Render_Object *object = &render_context.debug_render_objects[render_context.debug_render_object_count++];
	*object = (Debug_Render_Object){
		.index_count      = index_count,
		.color            = color,
		.render_primitive = primitive,
	};
	vulkan_push_debug_vertices(vertices, vertex_count, sizeof_vertex, indices, index_count, &object->vertex_offset, &object->first_index);
	ASSERT(render_context.debug_render_object_count < MAX_DEBUG_RENDER_OBJECTS);
}

void draw_wire_sphere(V3 center, f32 radius, V4 color) {
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
		xy = radius * cosf(stack_angle);         // r * cos(u)
		z = radius * sinf(stack_angle);          // r * sin(u)
		for (u32 j = 0; j <= sector_count; ++j) {
			sector_angle = j * sector_step; // From 0 to 2pi.
			x = xy * cosf(sector_angle);    // r * cos(u) * cos(v)
			y = xy * sinf(sector_angle);    // r * cos(u) * sin(v)
			vertices[vertex_count++].position = add_v3((V3){x, y, z}, center);
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
	add_debug_render_object(vertices, vertex_count, sizeof(vertices[0]), indices, index_count, color, LINE_PRIMITIVE);
}

void frustum_cull_meshes(Camera *camera, Bounding_Sphere *mesh_bounding_spheres, u32 mesh_bounding_sphere_count, f32 focal_length, f32 aspect_ratio, u32 *visible_meshes, u32 *visible_mesh_count) {
	f32 half_near_plane_height = render_context.focal_length * tan(camera->field_of_view / 2.0f);
	f32 half_near_plane_width = render_context.focal_length * tan(camera->field_of_view / 2.0f) * render_context.aspect_ratio;
	V3 center_of_near_plane = multiply_f32_v3(render_context.focal_length, camera->forward);
	enum {
		RIGHT,
		LEFT,
		TOP,
		BOTTOM,
		FRUSTUM_PLANE_COUNT,
	};
	V3 frustum_plane_normals[] = {
		[RIGHT]  = cross_product(normalize(add_v3(center_of_near_plane, multiply_f32_v3(half_near_plane_width, camera->side))), camera->up),
		[LEFT]   = cross_product(camera->up, normalize(subtract_v3(center_of_near_plane, multiply_f32_v3(half_near_plane_width, camera->side)))),
		[TOP]    = cross_product(camera->side, normalize(add_v3(center_of_near_plane, multiply_f32_v3(half_near_plane_height, camera->up)))),
		[BOTTOM] = cross_product(normalize(subtract_v3(center_of_near_plane, multiply_f32_v3(half_near_plane_height, camera->up))), camera->side),
	};
	for (u32 i = 0; i < mesh_bounding_sphere_count; i++) {
		draw_wire_sphere(mesh_bounding_spheres[i].center, mesh_bounding_spheres[i].radius, v3_to_v4(random_color_table[i % RANDOM_COLOR_TABLE_LENGTH], 1.0f));
		if ((dot_product(frustum_plane_normals[RIGHT], mesh_bounding_spheres[i].center) - dot_product(frustum_plane_normals[RIGHT], camera->position) <= mesh_bounding_spheres[i].radius)
		 && (dot_product(frustum_plane_normals[LEFT], mesh_bounding_spheres[i].center) - dot_product(frustum_plane_normals[LEFT], camera->position) <= mesh_bounding_spheres[i].radius)
		 && (dot_product(frustum_plane_normals[TOP], mesh_bounding_spheres[i].center) - dot_product(frustum_plane_normals[TOP], camera->position) <= mesh_bounding_spheres[i].radius)
		 && (dot_product(frustum_plane_normals[BOTTOM], mesh_bounding_spheres[i].center) - dot_product(frustum_plane_normals[BOTTOM], camera->position) <= mesh_bounding_spheres[i].radius)) {
		 	ASSERT(*visible_mesh_count < MAX_VISIBLE_ENTITY_MESHES);
			visible_meshes[(*visible_mesh_count)++] = i;
		}
	}
}

V3 random_color() {
	float r = rand() / (float)RAND_MAX;
	float g = rand() / (float)RAND_MAX;
	float b = rand() / (float)RAND_MAX;
	return (V3){r, g, b};
}

void initialize_renderer(Camera *camera, Memory_Arena *arena) {
	initialize_vulkan(arena);
	render_context.aspect_ratio = swapchain_image_width() / (f32)swapchain_image_height();
	render_context.focal_length = 0.1f;
	render_context.scene_projection = perspective_projection(camera->field_of_view, render_context.aspect_ratio, render_context.focal_length, 100.0f); // @TODO

	for (u32 i = 0; i < RANDOM_COLOR_TABLE_LENGTH; i++) {
		random_color_table[i] = random_color();
	}
}

void render(Game_State *game_state) {
	u32 *visible_meshes = allocate_array(&game_state->frame_arena, u32, MAX_VISIBLE_ENTITY_MESHES);
	u32 visible_mesh_count = 0;
	frustum_cull_meshes(&game_state->camera, game_state->entities.meshes.bounding_spheres, game_state->entities.meshes.count, render_context.focal_length, render_context.aspect_ratio, visible_meshes, &visible_mesh_count);
	vulkan_submit(&game_state->camera, game_state->entities.meshes.instances, visible_meshes, visible_mesh_count, &render_context); // @TODO
	render_context.debug_render_object_count = 0;
}
