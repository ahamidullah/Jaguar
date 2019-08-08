typedef enum {
	TRANSFORM_COMPONENT_ID,
    RENDER_COMPONENT_ID,
    MATERIAL_COMPONENT_ID,
} Component_ID;

void add_component() {
}

void initialize_components(Game_Components *components) {
	components->transform = malloc(sizeof(Transform_Component) * 1000);
	components->render = malloc(sizeof(Render_Component) * 1000);
	components->animation = malloc(sizeof(Animation_Component) * 1000);
	components->material = malloc(sizeof(Material_Component) * 1000);
}
