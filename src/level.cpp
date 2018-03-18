/***************************************************************************************************/
/*          Copyright (C) 2015-2017 By Yang Chen (yngccc@gmail.com). All Rights Reserved.          */
/***************************************************************************************************/

#pragma once

#include "../vendor/include/json/json.hpp"

#include "../vendor/include/bullet/btBulletCollisionCommon.h"
#include "../vendor/include/bullet/btBulletDynamicsCommon.h"

const uint32 level_max_entity_count = 1024;
const uint32 level_max_directional_light_count = 1;
const uint32 level_max_point_light_count = 4;
const uint32 level_max_spot_light_count = 4;

struct model_scene {
	uint32 node_indices[m_countof(gpk_model_scene::node_indices)];
	uint32 node_index_count;
	char name[sizeof(gpk_model_scene::name)];
};

struct model_node {
	uint32 mesh_index;
	mat4 local_transform_mat;
	uint32 children[32];
	uint32 child_count;
};

struct model_mesh {
	uint32 material_index;
	uint32 index_count;
	uint32 index_buffer_offset;
	uint32 vertex_count;
	uint32 vertex_buffer_offset;
	uint8 *vertices_data;
	uint8 *indices_data;
	char name[sizeof(gpk_model_mesh::name)];
};

struct model_joint {
	uint32 node_index;
	mat4 inverse_bind_mat;
};

struct model_skin {
	uint32 root_node_index;
	model_joint *joints;
	uint32 joint_count;
	char name[sizeof(gpk_model_skin::name)];
};

struct model_material {
	VkDescriptorSet textures_descriptor_set;
	vec4 albedo_factor;
	float metallic_factor;
	float roughness_factor;
	char name[sizeof(gpk_model_material::name)];
};

struct model_image {
	vulkan_image image;
};

struct model {
	model_scene *scenes;
	uint32 scene_count;
	model_node *nodes;
	uint32 node_count;
	model_mesh *meshes;
	uint32 mesh_count;
	model_skin *skins;
	uint32 skin_count;
	model_material *materials;
	uint32 material_count;
	model_image *images;
	uint32 image_count;
	char gpk_file[128];
};

struct skybox {
	VkDescriptorSet descriptor_set;
	vulkan_image cubemap_image;
	char gpk_file[128];
};

enum light_type {
	light_type_ambient,
	light_type_directional,
	light_type_point
};

struct ambient_light {
	vec3 color;
};

struct directional_light {
	vec3 color;
	vec3 direction;
};

struct point_light {
	vec3 color;
	vec3 position;
	float attenuation;
};

enum collision_shape {
	collision_shape_sphere,
	collision_shape_capsule,
	collision_shape_box
};

struct entity_info {
	char name[32];
};

enum entity_component_flag {
	entity_component_flag_render    = 1 << 0,
	entity_component_flag_collision = 1 << 1,
	entity_component_flag_physics   = 1 << 2,
	entity_component_flag_light     = 1 << 3
};

struct entity_render_component {
	uint32 model_index;
	transform adjustment_transform;
	bool hide;
};

struct entity_collision_component {
	collision_shape shape;
	union {
		struct {
			float radius;
		} sphere;
		struct {
			float height, radius;
		} capsule;
		struct {
			vec3 size;
		} box;
	};
	btCollisionObject *bt_collision_object;
};

struct entity_physics_component {
	vec3 velocity;
	float mass;
	float max_speed;
	btRigidBody *bt_rigid_body;
};

struct entity_light_component {
	light_type light_type;
	union {
		ambient_light ambient_light;
		directional_light directional_light;
		point_light point_light;
	};
};

struct entity_modification {
	bool remove;
	bool remove_render_component;
	bool remove_collision_component;
	bool remove_physics_component;
	bool remove_light_component;
	uint32 *flags;
	entity_info *info;
	transform *transform;
	entity_render_component *render_component;
	entity_collision_component *collision_component;
	entity_physics_component *physics_component;
	entity_light_component *light_component;
};

struct entity_addition {
	uint32 flag;
	entity_info info;
	transform transform;
	entity_render_component *render_component;
	entity_collision_component *collision_component;
	entity_physics_component *physics_component;
	entity_light_component *light_component;
	entity_addition *next;
};

struct mesh_render_data {
	uint32 mesh_index;
	uint32 frame_uniform_buffer_offset;
 	bool render_vertices_outline;
};

struct model_render_data {
	uint32 model_index;
	mesh_render_data *meshes;
	uint32 mesh_count;
};

struct level_render_data {
	uint32 common_data_frame_uniform_buffer_offset;
	uint32 text_frame_vertex_buffer_offset;
	uint32 text_frame_uniform_buffer_offset;
	uint32 text_frame_vertex_count;
	uint32 text_frame_uniform_count;
	model_render_data *models;
	uint32 model_count;
};

struct level {
	uint32 *entity_flags;
	entity_info *entity_infos;
	transform *entity_transforms;
	uint32 entity_count;

	entity_render_component *render_components;
	entity_collision_component *collision_components;
	entity_light_component *light_components;
	entity_physics_component *physics_components;
	uint32 render_component_count;
	uint32 collision_component_count;
	uint32 light_component_count;
	uint32 physics_component_count;

	entity_modification *entity_modifications;
	entity_addition *entity_addition;

	uint32 player_entity_index;

	model *models;
	uint32 model_count;
	uint32 model_capacity;

	skybox *skyboxes;
	uint32 skybox_count;
	uint32 skybox_capacity;
	uint32 skybox_index;

	level_render_data render_data;
	bool show_frame_stats;

	memory_arena main_thread_frame_memory_arena;
	memory_arena render_thread_frame_memory_arena;
	memory_arena entity_components_memory_arenas[2];
	uint32 entity_components_memory_arena_index;
	memory_arena assets_memory_arena;

	char json_file[128];
};

void initialize_level(level *level, vulkan *vulkan) {
	level->main_thread_frame_memory_arena.name = "main thread frame";
	level->main_thread_frame_memory_arena.capacity = m_megabytes(4);
	level->main_thread_frame_memory_arena.memory = allocate_virtual_memory(level->main_thread_frame_memory_arena.capacity);
	m_assert(level->main_thread_frame_memory_arena.memory);

	level->render_thread_frame_memory_arena.name = "render thread frame";
	level->render_thread_frame_memory_arena.capacity = m_megabytes(4);
	level->render_thread_frame_memory_arena.memory = allocate_virtual_memory(level->render_thread_frame_memory_arena.capacity);
	m_assert(level->render_thread_frame_memory_arena.memory);

	for (uint32 i = 0; i < m_countof(level->entity_components_memory_arenas); i += 1) {
		level->entity_components_memory_arenas[i].name = "entity_components";
		level->entity_components_memory_arenas[i].capacity = m_megabytes(8);
		level->entity_components_memory_arenas[i].memory = allocate_virtual_memory(level->entity_components_memory_arenas[i].capacity);
		m_assert(level->entity_components_memory_arenas[i].memory);
	}

	level->assets_memory_arena.name = "assets";
	level->assets_memory_arena.capacity = m_megabytes(64);
	level->assets_memory_arena.memory = allocate_virtual_memory(level->assets_memory_arena.capacity);
	m_assert(level->assets_memory_arena.memory);

	level->model_capacity = 1024;
	level->model_count = 0;
	level->models = allocate_memory<struct model>(&level->assets_memory_arena, level->model_capacity);

	level->skybox_capacity = 16;
	level->skybox_count = 0;
	level->skyboxes = allocate_memory<struct skybox>(&level->assets_memory_arena, level->skybox_capacity);
}

entity_render_component *entity_get_render_component(level *level, uint32 entity_index) {
	m_assert(entity_index < level->entity_count);
	m_assert(level->entity_flags[entity_index] & entity_component_flag_render);
	uint32 index = 0;
	for (uint32 i = 0; i < entity_index; i += 1) {
		if (level->entity_flags[i] & entity_component_flag_render) {
			index += 1;
		}
	}
	return &level->render_components[index];
}

entity_collision_component *entity_get_collision_component(level *level, uint32 entity_index) {
	m_assert(entity_index < level->entity_count);
	m_assert(level->entity_flags[entity_index] & entity_component_flag_collision);
	uint32 index = 0;
	for (uint32 i = 0; i < entity_index; i += 1) {
		if (level->entity_flags[i] & entity_component_flag_collision) {
			index += 1;
		}
	}
	return &level->collision_components[index];
}

entity_light_component *entity_get_light_component(level *level, uint32 entity_index) {
	m_assert(entity_index < level->entity_count);
	m_assert(level->entity_flags[entity_index] & entity_component_flag_light);
	uint32 index = 0;
	for (uint32 i = 0; i < entity_index; i += 1) {
		if (level->entity_flags[i] & entity_component_flag_light) {
			index += 1;
		}
	}
	return &level->light_components[index];
}

entity_physics_component *entity_get_physics_component(level *level, uint32 entity_index) {
	m_assert(entity_index < level->entity_count);
	m_assert(level->entity_flags[entity_index] & entity_component_flag_physics);
	uint32 index = 0;
	for (uint32 i = 0; i < entity_index; i += 1) {
		if (level->entity_flags[i] & entity_component_flag_physics) {
			index += 1;
		}
	}
	return &level->physics_components[index];
}

uint32 entity_component_get_entity_index(level *level, uint32 component_index, entity_component_flag entity_component_flag) {
	uint32 index = 0;
	for (uint32 i = 0; i < level->entity_count; i += 1) {
		if (level->entity_flags[i] & entity_component_flag) {
			if (index == component_index) {
				return i;
			}
			index += 1;
		}
	}
	m_assert(false);
	return UINT32_MAX;
}

#if 0
void level_update_json(level *level) {
	auto get_entity_json = [level](uint32 entity_index) {
		const char *entity_name = level->entity_infos[entity_index].name;
		auto &entities_json = level->json["entities"];
		for (auto e = entities_json.begin(); e != entities_json.end(); e += 1) {
			std::string name = (*e)["name"];
			if (name == entity_name) {
				return e;
			}
		}
		m_assert(false);
		return entities_json.end();
	};

	for (uint32 i = 0; i < level->entity_count; i += 1) {
		entity_modification *modification = &level->entity_modifications[i];
		if (modification->remove) {
			level->json["entities"].erase(get_entity_json(i));
		}
		else {
			if (modification->remove_render_component) {
				auto entity_json = get_entity_json(i);
				auto render_component = entity_json->find("render_component");
				m_assert(render_component != entity_json->end());
				entity_json->erase(render_component);
			}
			if (modification->remove_collision_component) {
				auto entity_json = get_entity_json(i);
				auto collision_component = entity_json->find("collision_component");
				m_assert(collision_component != entity_json->end());
				entity_json->erase(collision_component);
			}
			if (modification->remove_physics_component) {
				auto entity_json = get_entity_json(i);
				auto physics_component = entity_json->find("physics_component");
				m_assert(physics_component != entity_json->end());
				entity_json->erase(physics_component);
			}
			if (modification->remove_light_component) {
				auto entity_json = get_entity_json(i);
				auto light_component = entity_json->find("light_component");
				m_assert(light_component != entity_json->end());
				entity_json->erase(light_component);
			}
			if (modification->flags) {
			}
			if (modification->info) {
			}
			if (modification->transform) {
			}
		}
	}
}
#endif

void level_update_entity_components(level *level) {
	uint32 new_entity_count = level->entity_count;
	uint32 new_render_component_count = level->render_component_count;
	uint32 new_collision_component_count = level->collision_component_count;
	uint32 new_physics_component_count = level->physics_component_count;
	uint32 new_light_component_count = level->light_component_count;
	for (uint32 i = 0; i < level->entity_count; i += 1) {
		uint32 entity_flag = level->entity_flags[i];
		entity_modification *em = &level->entity_modifications[i];
		if (em->remove) {
			new_entity_count -= 1;
			if (entity_flag & entity_component_flag_render) {
				new_render_component_count -= 1;
			}
			if (entity_flag & entity_component_flag_collision) {
				new_collision_component_count -= 1;
			}
			if (entity_flag & entity_component_flag_physics) {
				new_physics_component_count -= 1;
			}
			if (entity_flag & entity_component_flag_light) {
				new_light_component_count -= 1;
			}
		}
		else {
			if (em->remove_render_component) {
				new_render_component_count -= 1;
			}
			else if (em->render_component && !(entity_flag & entity_component_flag_render)) {
				new_render_component_count += 1;
			}
			if (em->remove_collision_component) {
				new_collision_component_count -= 1;
			}
			else if (em->collision_component && !(entity_flag & entity_component_flag_collision)) {
				new_collision_component_count += 1;
			}
			if (em->remove_physics_component) {
				new_physics_component_count -= 1;
			}
			else if (em->physics_component && !(entity_flag & entity_component_flag_physics)) {
				new_physics_component_count += 1;
			}
			if (em->remove_light_component) {
				new_light_component_count -= 1;
			}
			else if (em->light_component && !(entity_flag & entity_component_flag_light)) {
				new_light_component_count += 1;
			}
		}
	}
	entity_addition *ea = level->entity_addition;
	while (ea) {
		new_entity_count += 1;
		if (ea->render_component) {
			new_render_component_count += 1;
		}
		if (ea->collision_component) {
			new_collision_component_count += 1;
		}
		if (ea->physics_component) {
			new_physics_component_count += 1;
		}
		if (ea->light_component) {
			new_light_component_count += 1;
		}
		ea = ea->next;
	}

	level->entity_components_memory_arena_index = (level->entity_components_memory_arena_index + 1) % 2;
	memory_arena *entity_components_memory_arena = &level->entity_components_memory_arenas[level->entity_components_memory_arena_index];
	entity_components_memory_arena->size = 0;

	uint32 *new_entity_flags = new_entity_count > 0 ? allocate_memory<uint32>(entity_components_memory_arena, new_entity_count) : nullptr;
	entity_info *new_entity_infos = new_entity_count > 0 ? allocate_memory<entity_info>(entity_components_memory_arena, new_entity_count) : nullptr;
	transform *new_entity_transforms = new_entity_count > 0 ? allocate_memory<transform>(entity_components_memory_arena, new_entity_count) : nullptr;
	entity_modification *new_entity_modifications = new_entity_count > 0 ? allocate_memory<entity_modification>(entity_components_memory_arena, new_entity_count) : nullptr;
	entity_render_component *new_render_components = new_render_component_count > 0 ? allocate_memory<entity_render_component>(entity_components_memory_arena, new_render_component_count) : nullptr;
	entity_collision_component *new_collision_components = new_collision_component_count > 0 ? allocate_memory<entity_collision_component>(entity_components_memory_arena, new_collision_component_count) : nullptr;
	entity_physics_component *new_physics_components = new_physics_component_count > 0 ? allocate_memory<entity_physics_component>(entity_components_memory_arena, new_physics_component_count) : nullptr;
	entity_light_component *new_light_components = new_light_component_count > 0 ? allocate_memory<entity_light_component>(entity_components_memory_arena, new_light_component_count) : nullptr;

	uint32 entity_index = 0;
	uint32 render_component_index = 0;
	uint32 collision_component_index = 0;
	uint32 physics_component_index = 0;
	uint32 light_component_index = 0;
	for (uint32 i = 0; i < level->entity_count; i += 1) {
		entity_modification *em = &level->entity_modifications[i];
		if (!em->remove) {
			new_entity_flags[entity_index] = em->flags ? *em->flags : level->entity_flags[i];
			new_entity_infos[entity_index] = em->info ? *em->info : level->entity_infos[i];
			new_entity_transforms[entity_index] = em->transform ? *em->transform : level->entity_transforms[i];
			if (!em->remove_render_component) {
				if (level->entity_flags[i] & entity_component_flag_render || em->render_component) {
					new_render_components[render_component_index++] = em->render_component ? *em->render_component : *entity_get_render_component(level, i);
				}
			}
			if (!em->remove_collision_component) {
				if (level->entity_flags[i] & entity_component_flag_collision || em->collision_component) {
					new_collision_components[collision_component_index++] = em->collision_component ? *em->collision_component : *entity_get_collision_component(level, i);
				}
			}
			if (!em->remove_physics_component) {
				if (level->entity_flags[i] & entity_component_flag_physics || em->physics_component) {
					new_physics_components[physics_component_index++] = em->physics_component ? *em->physics_component : *entity_get_physics_component(level, i);
				}
			}
			if (!em->remove_light_component) {
				if (level->entity_flags[i] & entity_component_flag_light || em->light_component) {
					new_light_components[light_component_index++] = em->light_component ? *em->light_component : *entity_get_light_component(level, i);
				}
			}
			entity_index += 1;
		}
	}
	ea = level->entity_addition;
	while (ea) {
		new_entity_flags[entity_index] = ea->flag;
		new_entity_infos[entity_index] = ea->info;
		new_entity_transforms[entity_index] = ea->transform;
		if (ea->render_component) {
			new_render_components[render_component_index++] = *ea->render_component;
		}
		if (ea->collision_component) {
			new_collision_components[collision_component_index++] = *ea->collision_component;
		}
		if (ea->physics_component) {
			new_physics_components[physics_component_index++] = *ea->physics_component;
		}
		if (ea->light_component) {
			new_light_components[light_component_index++] = *ea->light_component;
		}
		entity_index += 1;
		ea = ea->next;
	}

	level->entity_flags = new_entity_flags;
	level->entity_infos = new_entity_infos;
	level->entity_transforms = new_entity_transforms;
	level->entity_count = new_entity_count;

	level->entity_modifications = new_entity_modifications;
	level->entity_addition = nullptr;

	level->render_components = new_render_components;
	level->collision_components = new_collision_components;
	level->physics_components = new_physics_components;
	level->light_components = new_light_components;

	level->render_component_count = new_render_component_count;
	level->collision_component_count = new_collision_component_count;
	level->physics_component_count = new_physics_component_count;
	level->light_component_count = new_light_component_count;
}

uint32 level_get_model_index(level *level, const char *gpk_file) {
	for (uint32 i = 0; i < level->model_count; i += 1) {
		if (!strcmp(level->models[i].gpk_file, gpk_file)) {
			return i;
		}
	}
	return UINT32_MAX;
}

uint32 level_add_gpk_model(level *level, vulkan *vulkan, const char *gpk_file, bool store_vertices = false) {
	m_assert(level->model_count < level->model_capacity);
	for (uint32 i = 0; i < level->model_count; i += 1) {
		m_assert(strcmp(level->models[i].gpk_file, gpk_file));
	}

	file_mapping gpk_file_mapping = {};
	m_assert(open_file_mapping(gpk_file, &gpk_file_mapping));
	m_scope_exit(close_file_mapping(gpk_file_mapping));

	model *model = &level->models[level->model_count];
	gpk_model *gpk_model = (struct gpk_model *)gpk_file_mapping.ptr;
	snprintf(model->gpk_file, sizeof(model->gpk_file), "%s", gpk_file);
	model->scene_count = gpk_model->scene_count;
	model->node_count = gpk_model->node_count;
	model->mesh_count = gpk_model->mesh_count;
	model->skin_count = gpk_model->skin_count;
	model->material_count = gpk_model->material_count;
	model->image_count = gpk_model->image_count;
	model->scenes = allocate_memory<struct model_scene>(&level->assets_memory_arena, model->scene_count);
	model->nodes = allocate_memory<struct model_node>(&level->assets_memory_arena, model->node_count);
	model->meshes = allocate_memory<struct model_mesh>(&level->assets_memory_arena, model->mesh_count);
	if (model->skin_count > 0) {
		model->skins = allocate_memory<struct model_skin>(&level->assets_memory_arena, model->skin_count);
	}
	if (model->material_count > 0) {
		model->materials = allocate_memory<struct model_material>(&level->assets_memory_arena, model->material_count);
	}
	if (model->image_count > 0) {
		model->images = allocate_memory<struct model_image>(&level->assets_memory_arena, model->image_count);
	}
	for (uint32 i = 0; i < model->scene_count; i += 1) {
		gpk_model_scene *gpk_model_scene = ((struct gpk_model_scene*)(gpk_file_mapping.ptr + gpk_model->scene_offset)) + i;
		model_scene *model_scene = &model->scenes[i];

		array_copy(model_scene->name, gpk_model_scene->name);
		array_copy(model_scene->node_indices, gpk_model_scene->node_indices);
		model_scene->node_index_count = gpk_model_scene->node_index_count;
	}
	for (uint32 i = 0; i < model->node_count; i += 1) {
		gpk_model_node *gpk_model_node = ((struct gpk_model_node*)(gpk_file_mapping.ptr + gpk_model->node_offset)) + i;
		model_node *model_node = &model->nodes[i];

		model_node->mesh_index = gpk_model_node->mesh_index;
		model_node->local_transform_mat = gpk_model_node->local_transform_mat;
		array_copy(model_node->children, gpk_model_node->children);
		model_node->child_count = gpk_model_node->child_count;
	}
	for (uint32 i = 0; i < model->mesh_count; i += 1) {
		gpk_model_mesh *gpk_model_mesh = ((struct gpk_model_mesh*)(gpk_file_mapping.ptr + gpk_model->mesh_offset)) + i;
		model_mesh *model_mesh = &model->meshes[i];
		model_mesh->material_index = gpk_model_mesh->material_index;
		array_copy(model_mesh->name, gpk_model_mesh->name);

		uint32 indices_size = gpk_model_mesh->index_count * sizeof(uint16);
		uint32 vertices_size = gpk_model_mesh->vertex_count * sizeof(struct gpk_model_vertex);
		uint8 *indices_data = gpk_file_mapping.ptr + gpk_model_mesh->indices_offset;
		uint8 *vertices_data = gpk_file_mapping.ptr + gpk_model_mesh->vertices_offset;
			
		model_mesh->index_count = gpk_model_mesh->index_count;
		round_up(&vulkan->buffers.level_vertex_buffer_offset, (uint32)sizeof(uint16));
		model_mesh->index_buffer_offset = vulkan->buffers.level_vertex_buffer_offset;
		vulkan->buffers.level_vertex_buffer_offset += indices_size;

		model_mesh->vertex_count = gpk_model_mesh->vertex_count;
		round_up(&vulkan->buffers.level_vertex_buffer_offset, (uint32)sizeof(struct gpk_model_vertex));
		model_mesh->vertex_buffer_offset = vulkan->buffers.level_vertex_buffer_offset;
		vulkan->buffers.level_vertex_buffer_offset += vertices_size;

		vulkan_buffer_transfer(vulkan, &vulkan->buffers.level_vertex_buffer, model_mesh->index_buffer_offset, indices_data, indices_size);
		vulkan_buffer_transfer(vulkan, &vulkan->buffers.level_vertex_buffer, model_mesh->vertex_buffer_offset, vertices_data, vertices_size);
		if (store_vertices) {
			model_mesh->indices_data = allocate_memory<uint8>(&level->assets_memory_arena, indices_size);
			model_mesh->vertices_data = allocate_memory<uint8>(&level->assets_memory_arena, vertices_size);
			memcpy(model_mesh->indices_data, indices_data, indices_size);
			memcpy(model_mesh->vertices_data, vertices_data, vertices_size);
		}
	}
	for (uint32 i = 0; i < model->skin_count; i += 1) {
		gpk_model_skin *gpk_model_skin = ((struct gpk_model_skin*)(gpk_file_mapping.ptr + gpk_model->skin_offset)) + i;
		model_skin *model_skin = &model->skins[i];
		array_copy(model_skin->name, gpk_model_skin->name);
		model_skin->root_node_index = gpk_model_skin->root_node_index;
		model_skin->joint_count = gpk_model_skin->joint_count;
		m_assert(model_skin->joint_count > 0);
		model_skin->joints = allocate_memory<struct model_joint>(&level->assets_memory_arena, model_skin->joint_count);
		gpk_model_joint *gpk_joints = (gpk_model_joint *)(gpk_file_mapping.ptr + gpk_model_skin->joints_offset);
		for (uint32 i = 0; i < model_skin->joint_count; i += 1) {
			model_skin->joints[i].node_index = gpk_joints[i].node_index;
			model_skin->joints[i].inverse_bind_mat = gpk_joints[i].inverse_bind_mat;
		}
	}
	for (uint32 i = 0; i < model->image_count; i += 1) {
		gpk_model_image *gpk_model_image = ((struct gpk_model_image*)(gpk_file_mapping.ptr + gpk_model->image_offset)) + i;
		VkImageCreateInfo image_create_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
		image_create_info.imageType = VK_IMAGE_TYPE_2D;
		image_create_info.format = (VkFormat)gpk_model_image->format;
		image_create_info.extent = {gpk_model_image->width, gpk_model_image->height, 1};
		image_create_info.mipLevels = gpk_model_image->mipmap_count;
		image_create_info.arrayLayers = gpk_model_image->layer_count;
		image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
		image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
		image_create_info.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		VkImageViewCreateInfo image_view_create_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
		image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		image_view_create_info.format = image_create_info.format;
		image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		image_view_create_info.subresourceRange.levelCount = image_create_info.mipLevels;
		image_view_create_info.subresourceRange.layerCount = image_create_info.arrayLayers;
		vulkan_image vulkan_image = allocate_vulkan_image(&vulkan->device, &vulkan->memories.level_images_memory, image_create_info, image_view_create_info, gpk_model_image->format_block_dimension, gpk_model_image->format_block_size);
		vulkan_image_transfer(&vulkan->device, &vulkan->cmd_buffers, &vulkan_image, gpk_file_mapping.ptr + gpk_model_image->data_offset, gpk_model_image->size);
		model->images[i].image = vulkan_image;
	}
	for (uint32 i = 0; i < model->material_count; i += 1) {
		gpk_model_material *gpk_model_material = ((struct gpk_model_material*)(gpk_file_mapping.ptr + gpk_model->material_offset)) + i;
		model_material *model_material = &model->materials[i];
		array_copy(model_material->name, gpk_model_material->name);
		model_material->albedo_factor = gpk_model_material->albedo_factor;
		model_material->metallic_factor = gpk_model_material->metallic_factor;
		model_material->roughness_factor = gpk_model_material->roughness_factor;

		VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
		descriptor_set_allocate_info.descriptorPool = vulkan->descriptors.pool;
		descriptor_set_allocate_info.descriptorSetCount = 1;
		descriptor_set_allocate_info.pSetLayouts = &vulkan->descriptors.textures_descriptor_set_layouts[4];
		m_vk_assert(vkAllocateDescriptorSets(vulkan->device.device, &descriptor_set_allocate_info, &model_material->textures_descriptor_set));
		VkDescriptorImageInfo descriptor_image_infos[5] = {};
		if (gpk_model_material->albedo_image_index < model->image_count) {
			vulkan_image *image = &model->images[gpk_model_material->albedo_image_index].image;
			descriptor_image_infos[0] = {vulkan->samplers.mipmap_samplers[image->mipmap_count], image->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		}
		else {
			descriptor_image_infos[0] = {vulkan->samplers.mipmap_samplers[1], vulkan->images.default_albedo_map_image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		}
		if (gpk_model_material->metallic_image_index < model->image_count) {
			vulkan_image *image = &model->images[gpk_model_material->metallic_image_index].image;
			descriptor_image_infos[1] = {vulkan->samplers.mipmap_samplers[image->mipmap_count], image->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		}
		else {
			descriptor_image_infos[1] = {vulkan->samplers.mipmap_samplers[1], vulkan->images.default_metallic_map_image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		}
		if (gpk_model_material->roughness_image_index < model->image_count) {
			vulkan_image *image = &model->images[gpk_model_material->roughness_image_index].image;
			descriptor_image_infos[2] = {vulkan->samplers.mipmap_samplers[image->mipmap_count], image->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		}
		else {
			descriptor_image_infos[2] = {vulkan->samplers.mipmap_samplers[1], vulkan->images.default_roughness_map_image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		}
		if (gpk_model_material->normal_image_index < model->image_count) {
			vulkan_image *image = &model->images[gpk_model_material->normal_image_index].image;
			descriptor_image_infos[3] = {vulkan->samplers.mipmap_samplers[image->mipmap_count], image->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		}
		else {
			descriptor_image_infos[3] = {vulkan->samplers.mipmap_samplers[1], vulkan->images.default_normal_map_image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		}
		// if (gpk_model_material->height_image_index < model->image_count) {
		// 	vulkan_image *image = &model->images[gpk_model_material->height_image_index].image;
		// 	descriptor_image_infos[4] = {vulkan->samplers.mipmap_image_samplers[image->mipmap_count], image->view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		// }
		// else {
		// 	descriptor_image_infos[4] = {vulkan->samplers.mipmap_image_samplers[1], vulkan->images.default_height_map_image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		// }
		descriptor_image_infos[4] = {vulkan->samplers.mipmap_samplers[1], vulkan->images.default_height_map_image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
		VkWriteDescriptorSet write_descriptor_sets[5] = {};
		for (uint32 i = 0; i < m_countof(write_descriptor_sets); i += 1) {
			write_descriptor_sets[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_descriptor_sets[i].dstSet = model_material->textures_descriptor_set;
			write_descriptor_sets[i].dstBinding = i;
			write_descriptor_sets[i].descriptorCount = 1;
			write_descriptor_sets[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_descriptor_sets[i].pImageInfo = &descriptor_image_infos[i];
		}
		vkUpdateDescriptorSets(vulkan->device.device, m_countof(write_descriptor_sets), write_descriptor_sets, 0, nullptr);
	}
	return level->model_count++;
}

void level_add_skybox(level *level, vulkan *vulkan, const char *gpk_file) {
	m_assert(level->skybox_count < level->skybox_capacity);
	skybox *skybox = &level->skyboxes[level->skybox_count++];
	snprintf(skybox->gpk_file, sizeof(skybox->gpk_file), "%s", gpk_file);

	file_mapping file_mapping = {};
	m_assert(open_file_mapping(gpk_file, &file_mapping));
	m_scope_exit(close_file_mapping(file_mapping));
	gpk_skybox *gpk_skybox = (struct gpk_skybox *)file_mapping.ptr;

	VkImageCreateInfo image_create_info = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
	image_create_info.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	image_create_info.imageType = VK_IMAGE_TYPE_2D;
	image_create_info.format = VK_FORMAT_R8G8B8A8_SRGB;
	image_create_info.extent = {gpk_skybox->cubemap_width, gpk_skybox->cubemap_height, 1};
	image_create_info.mipLevels = 1;
	image_create_info.arrayLayers = 6;
	image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_create_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_create_info.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	VkImageViewCreateInfo image_view_create_info = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	image_view_create_info.format = image_create_info.format;
	image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	image_view_create_info.subresourceRange.levelCount = 1;
	image_view_create_info.subresourceRange.layerCount = 6;
	uint8 *cubemap = (uint8 *)gpk_skybox + gpk_skybox->cubemap_offset;
	uint32 cubemap_size = gpk_skybox->cubemap_width * gpk_skybox->cubemap_height * 4 * 6;
	skybox->cubemap_image = allocate_vulkan_image(&vulkan->device, &vulkan->memories.level_images_memory, image_create_info, image_view_create_info, 1, 4);
	vulkan_image_transfer(&vulkan->device, &vulkan->cmd_buffers, &skybox->cubemap_image, cubemap, cubemap_size);

	VkDescriptorSetAllocateInfo descriptor_set_allocate_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
	descriptor_set_allocate_info.descriptorPool = vulkan->descriptors.pool;
	descriptor_set_allocate_info.descriptorSetCount = 1;
	descriptor_set_allocate_info.pSetLayouts = &vulkan->descriptors.textures_descriptor_set_layouts[0];
	m_vk_assert(vkAllocateDescriptorSets(vulkan->device.device, &descriptor_set_allocate_info, &skybox->descriptor_set));
	VkDescriptorImageInfo descriptor_image_info = {vulkan->samplers.skybox_cubemap_sampler, skybox->cubemap_image.view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL};
	VkWriteDescriptorSet write_descriptor_set = {VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET};
	write_descriptor_set.dstSet = skybox->descriptor_set;
	write_descriptor_set.dstBinding = 0;
	write_descriptor_set.descriptorCount = 1;
	write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	write_descriptor_set.pImageInfo = &descriptor_image_info;
	vkUpdateDescriptorSets(vulkan->device.device, 1, &write_descriptor_set, 0, nullptr);
}

template <typename F>
void level_read_json(level *level, vulkan *vulkan, const char *level_json_file, F extra_read, bool store_vertices) {
	file_mapping level_json_file_mapping = {};
	m_assert(open_file_mapping(level_json_file, &level_json_file_mapping));
	nlohmann::json json = nlohmann::json::parse(level_json_file_mapping.ptr, level_json_file_mapping.ptr + level_json_file_mapping.size);
	close_file_mapping(level_json_file_mapping);
	{ // models, skyboxes
		std::vector<std::string> model_gpk_files = json["models"];
		for (auto &m : model_gpk_files) {
			level_add_gpk_model(level, vulkan, m.c_str(), store_vertices);
		}
		std::vector<std::string> skybox_gpk_files = json["skyboxes"];
		for (auto &s : skybox_gpk_files) {
			level_add_skybox(level, vulkan, s.c_str());
		}
		level->skybox_index = json["skybox_index"];
	}
	{ // entities
		auto &entities_json = json["entities"];
		level->entity_count = (uint32)entities_json.size();
		level->render_component_count = 0;
		level->collision_component_count = 0;
		level->physics_component_count = 0;
		level->light_component_count = 0;
		for (auto &entity : entities_json) {
			if (entity.find("render_component") != entity.end()) {
				level->render_component_count += 1;
			}
			if (entity.find("collision_component") != entity.end()) {
				level->collision_component_count += 1;
			}
			if (entity.find("physics_component") != entity.end()) {
				level->physics_component_count += 1;
			}
			if (entity.find("light_component") != entity.end()) {
				level->light_component_count += 1;
			}
		}

		level->entity_components_memory_arena_index = 0;
		memory_arena *memory_arena = &level->entity_components_memory_arenas[0];
		level->entity_flags = allocate_memory<uint32>(memory_arena, level->entity_count);
		level->entity_infos = allocate_memory<entity_info>(memory_arena, level->entity_count);
		level->entity_transforms = allocate_memory<transform>(memory_arena, level->entity_count);
		level->entity_modifications = allocate_memory<entity_modification>(memory_arena, level->entity_count);
		level->render_components = allocate_memory<entity_render_component>(memory_arena, level->render_component_count);
		level->collision_components = allocate_memory<entity_collision_component>(memory_arena, level->collision_component_count);
		level->physics_components = allocate_memory<entity_physics_component>(memory_arena, level->physics_component_count);
		level->light_components = allocate_memory<entity_light_component>(memory_arena, level->light_component_count);

		auto read_entity_info = [](const nlohmann::json &json, entity_info *info) {
			std::string name = json["name"];
			m_assert(name.length() < sizeof(info->name));
			strcpy(info->name, name.c_str());
		};
		auto read_transform = [](const nlohmann::json &json, transform *transform) {
			std::array<float, 3> scale = json["scale"];
			std::array<float, 4> rotate = json["rotate"];
			std::array<float, 3> translate = json["translate"];
			transform->scale = {scale[0], scale[1], scale[2]};
			transform->rotate = {rotate[0], rotate[1], rotate[2], rotate[3]};
			transform->translate = {translate[0], translate[1], translate[2]};
		};
    auto read_render_component = [level, read_transform](const nlohmann::json &json, entity_render_component *render_component) {
			std::string gpk_file = json["gpk_file"];
			render_component->model_index = level_get_model_index(level, gpk_file.c_str());
			auto adjustment_transform_field = json.find("adjustment_transform");
			if (adjustment_transform_field != json.end()) {
				read_transform(*adjustment_transform_field, &render_component->adjustment_transform);
			}
			else {
				render_component->adjustment_transform = transform_identity();
			}
		};
    auto read_collision_component = [](const nlohmann::json &json, entity_collision_component *collision_component) {
			std::string shape = json["shape"];
			if (shape == "sphere") {
				collision_component->shape = collision_shape_sphere;
				collision_component->sphere.radius = json["radius"];
			}
			else if (shape == "capsule") {
				collision_component->shape = collision_shape_capsule;
				collision_component->capsule.height = json["height"];
				collision_component->capsule.radius = json["radius"];
			}
			else if (shape == "box") {
				collision_component->shape = collision_shape_box;
				std::array<float, 3> size = json["size"];
				collision_component->box.size = {size[0], size[1], size[2]};
			}
			else {
				m_assert(false);
			}
		};
		auto read_physics_component = [](const nlohmann::json &json, entity_physics_component *physics_component) {
			auto velocity_field = json.find("velocity");
			if (velocity_field != json.end()) {
				std::array<float, 3> velocity = *velocity_field;
				physics_component->velocity = {velocity[0], velocity[1], velocity[2]};
			}
			auto mass_field = json.find("mass");
			if (mass_field != json.end()) {
				physics_component->mass = *mass_field;
			}
			auto max_speed_field = json.find("max_speed");
			if (max_speed_field != json.end()) {
				physics_component->max_speed = *max_speed_field;
			}
		};
		auto read_light_component = [](const nlohmann::json &json, entity_light_component *light_component) {
			std::string light_type = json["light_type"];
			if (light_type == "ambient") {
				light_component->light_type = light_type_ambient;
				std::array<float, 3> color = json["color"];
				light_component->ambient_light.color = {color[0], color[1], color[2]};
			}
			else if (light_type == "directional") {
				light_component->light_type = light_type_directional;
				std::array<float, 3> color = json["color"];
				std::array<float, 3> direction = json["direction"];
				light_component->directional_light.color = {color[0], color[1], color[2]};
				light_component->directional_light.direction = {direction[0], direction[1], direction[2]};
			}
			else if (light_type == "point") {
				light_component->light_type = light_type_point;
				std::array<float, 3> color = json["color"];
				std::array<float, 3> position = json["position"];
				light_component->point_light.color = {color[0], color[1], color[2]};
				light_component->point_light.position = {position[0], position[1], position[2]};
				light_component->point_light.attenuation = json["attenuation"];
			}
			else {
				m_assert(false);
			}
		};

		uint32 render_component_index = 0;
		uint32 collision_component_index = 0;
		uint32 physics_component_index = 0;
		uint32 light_component_index = 0;
		for (uint32 i = 0; i < level->entity_count; i += 1) {
			auto &entity_json = entities_json[i];
			uint32 &entity_flags = level->entity_flags[i];
			entity_info &entity_info = level->entity_infos[i];
			transform &entity_transform = level->entity_transforms[i];
			entity_flags = 0;
			read_entity_info(entity_json, &entity_info);
			read_transform(entity_json["transform"], &entity_transform);

			auto render_component_field = entity_json.find("render_component");
			if (render_component_field != entity_json.end()) {
				entity_flags |= entity_component_flag_render;
				read_render_component(*render_component_field, &level->render_components[render_component_index++]);
			}
			auto collision_component_field = entity_json.find("collision_component");
			if (collision_component_field != entity_json.end()) {
				entity_flags |= entity_component_flag_collision;
				read_collision_component(*collision_component_field, &level->collision_components[collision_component_index++]);
			}
			auto physics_component_field = entity_json.find("physics_component");
			if (physics_component_field != entity_json.end()) {
				entity_flags |= entity_component_flag_physics;
				read_physics_component(*physics_component_field, &level->physics_components[physics_component_index++]);
			}
			auto light_component_field = entity_json.find("light_component");
			if (light_component_field != entity_json.end()) {
				entity_flags |= entity_component_flag_light;
				read_light_component(*light_component_field, &level->light_components[light_component_index++]);
			}
			if (collision_component_field != entity_json.end() && physics_component_field != entity_json.end()) {
				entity_collision_component *collision_component = &level->collision_components[collision_component_index - 1];
				entity_physics_component *physics_component = &level->physics_components[physics_component_index - 1];
				btRigidBody *rigid_body = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(physics_component->mass, nullptr, nullptr));
				if (collision_component->shape == collision_shape_sphere) {
					rigid_body->setCollisionShape(new btSphereShape(collision_component->sphere.radius));
				}
				else if (collision_component->shape == collision_shape_capsule) {
					rigid_body->setCollisionShape(new btCapsuleShape(collision_component->capsule.radius, collision_component->capsule.height));
				}
				else if (collision_component->shape == collision_shape_box) {
					rigid_body->setCollisionShape(new btBoxShape(btVector3(collision_component->box.size.x / 2, collision_component->box.size.y / 2, collision_component->box.size.z / 2)));
				}
				btQuaternion rotate(entity_transform.rotate.x, entity_transform.rotate.y, entity_transform.rotate.z, entity_transform.rotate.w);
				btVector3 translate(entity_transform.translate.x, entity_transform.translate.y, entity_transform.translate.z);
				rigid_body->setWorldTransform(btTransform(rotate, translate));
				rigid_body->setLinearVelocity(btVector3(physics_component->velocity.x, physics_component->velocity.y, physics_component->velocity.z));
				physics_component->bt_rigid_body = rigid_body;
			}
			else if (collision_component_field != entity_json.end()) {
				entity_collision_component *collision_component = &level->collision_components[collision_component_index - 1];
				btCollisionObject *collision_object = new btCollisionObject();
				if (collision_component->shape == collision_shape_sphere) {
					collision_object->setCollisionShape(new btSphereShape(collision_component->sphere.radius));
				}
				else if (collision_component->shape == collision_shape_capsule) {
					collision_object->setCollisionShape(new btCapsuleShape(collision_component->capsule.radius, collision_component->capsule.height));
				}
				else if (collision_component->shape == collision_shape_box) {
					collision_object->setCollisionShape(new btBoxShape(btVector3(collision_component->box.size.x / 2, collision_component->box.size.y / 2, collision_component->box.size.z / 2)));
				}
				btQuaternion rotate(entity_transform.rotate.x, entity_transform.rotate.y, entity_transform.rotate.z, entity_transform.rotate.w);
				btVector3 translate(entity_transform.translate.x, entity_transform.translate.y, entity_transform.translate.z);
				collision_object->setWorldTransform(btTransform(rotate, translate));
				collision_component->bt_collision_object = collision_object;
			}
			else if (physics_component_field != entity_json.end()) {
				entity_physics_component *physics_component = &level->physics_components[physics_component_index - 1];
				btRigidBody *rigid_body = new btRigidBody(btRigidBody::btRigidBodyConstructionInfo(physics_component->mass, nullptr, new btEmptyShape()));
				btQuaternion rotate(entity_transform.rotate.x, entity_transform.rotate.y, entity_transform.rotate.z, entity_transform.rotate.w);
				btVector3 translate(entity_transform.translate.x, entity_transform.translate.y, entity_transform.translate.z);
				rigid_body->setWorldTransform(btTransform(rotate, translate));
				rigid_body->setLinearVelocity(btVector3(physics_component->velocity.x, physics_component->velocity.y, physics_component->velocity.z));
				physics_component->bt_rigid_body = rigid_body;
			}
		}
	}
  { // player
		level->player_entity_index = UINT32_MAX;
		std::string player_entity_name = json["player"]["entity_name"];
		for (uint32 i = 0; i < level->entity_count; i += 1) {
			if (!strcmp(player_entity_name.c_str(), level->entity_infos[i].name)) {
				level->player_entity_index = i;
				break;
			}
		}
		m_assert(level->player_entity_index != UINT32_MAX);
	}
	snprintf(level->json_file, sizeof(level->json_file), "%s", level_json_file);
	extra_read(json);
}

template <typename F>
void level_write_json(level *level, const char *json_file_path, F extra_write) {
	nlohmann::json json;
	{ // models, skyboxes
		auto &models = json["models"];
		for (uint32 i = 0; i < level->model_count; i += 1) {
			models.push_back(level->models[i].gpk_file);
		}
		auto &skyboxes = json["skyboxes"];
		for (uint32 i = 0; i < level->skybox_count; i += 1) {
			skyboxes.push_back(level->skyboxes[i].gpk_file);
		}
		json["skybox_index"] = level->skybox_index;
	}
	{ // entities
		auto &entities = json["entities"];
		for (uint32 i = 0; i < level->entity_count; i += 1) {
			uint32 &flags = level->entity_flags[i];
			entity_info &info = level->entity_infos[i];
			transform &tfm = level->entity_transforms[i];
			nlohmann::json entity_json = {
				{"name", level->entity_infos[i].name},
				{"transform", {
					{"scale", {m_unpack3(tfm.scale)}}, 
					{"rotate", {m_unpack4(tfm.rotate)}},
					{"translate", {m_unpack3(tfm.translate)}}}
				}
			};
			if (flags & entity_component_flag_render) {
				entity_render_component *render_component = entity_get_render_component(level, i);
				transform &tfm = render_component->adjustment_transform;
				entity_json["render_component"] = {
					{"gpk_file", level->models[render_component->model_index].gpk_file},
					{"adjustment_transform", {
						{"scale", {m_unpack3(tfm.scale)}}, 
						{"rotate", {m_unpack4(tfm.rotate)}},
						{"translate", {m_unpack3(tfm.translate)}}}
					}
				};
			}
			if (flags & entity_component_flag_collision) {
				entity_collision_component *collision_component = entity_get_collision_component(level, i);
				if (collision_component->shape == collision_shape_sphere) {
					entity_json["collision_component"] = {
						{"shape", "sphere"},
						{"radius", collision_component->sphere.radius}
					};
				}
				else if (collision_component->shape == collision_shape_capsule) {
					entity_json["collision_component"] = {
						{"shape", "capsule"},
						{"height", collision_component->capsule.height},
						{"radius", collision_component->capsule.radius}
					};
				}
				else if (collision_component->shape == collision_shape_box) {
					entity_json["collision_component"] = {
						{"shape", "box"},
						{"size", {m_unpack3(collision_component->box.size)}}
					};
				}
				else {
					m_assert(false);
				}
			}
			if (flags & entity_component_flag_physics) {
				entity_physics_component *physics_component = entity_get_physics_component(level, i);
				entity_json["physics_component"] = {
					{"velocity", {m_unpack3(physics_component->velocity)}},
					{"mass", physics_component->mass},
					{"max_speed", physics_component->max_speed}
				};
			}
			if (flags & entity_component_flag_light) {
				entity_light_component *light_component = entity_get_light_component(level, i);
				if (light_component->light_type == light_type_ambient) {
					entity_json["light_component"] = {
						{"light_type", "ambient"},
						{"color", {m_unpack3(light_component->ambient_light.color)}}
					};
				}				
				else if (light_component->light_type == light_type_directional) {
					entity_json["light_component"] = {
						{"light_type", "directional"},
						{"color", {m_unpack3(light_component->directional_light.color)}},
						{"direction", {m_unpack3(light_component->directional_light.direction)}}
					};
				}				
				else if (light_component->light_type == light_type_point) {
					entity_json["light_component"] = {
						{"light_type", "point"},
						{"color", {m_unpack3(light_component->point_light.color)}},
						{"position", {m_unpack3(light_component->point_light.position)}},
						{"attenuation", light_component->point_light.attenuation}
					};
				}				
			}
			entities.push_back(entity_json);
		}
	}
	{ // player
		json["player"] = {
			{"entity_name", level->entity_infos[level->player_entity_index].name}
		};
	}
	extra_write(json);
	std::string json_string = json.dump(2);

	file_mapping file_mapping = {};
	create_file_mapping(json_file_path, json_string.length(), &file_mapping);
	memcpy(file_mapping.ptr, json_string.c_str(), json_string.length());
	close_file_mapping(file_mapping);
}

camera level_get_player_camera(level *level, vulkan *vulkan, float r, float theta, float phi) {
	vec3 center = {};
	vec3 translate = {};
	if (level->player_entity_index < level->entity_count) {
		transform *transform = &level->entity_transforms[level->player_entity_index];
		center = transform->translate;
		quat rotate_0 = quat_from_rotation(vec3{1, 0, 0}, theta);
		quat rotate_1 = quat_from_rotation(vec3{0, 1, 0}, phi);
		translate = rotate_1 * rotate_0 * vec3{0, 0, -r};
	}
	else {
		center = {0, 0, 0};
		translate = vec3_normalize({0, 1, -1}) * r;
	}
	camera camera = {};
	camera.position = center + translate;
	camera.view = vec3_normalize(-translate);
	camera.up = vec3_cross(vec3_cross(camera.view, vec3{0, 1, 0}), camera.view);
	camera.fovy = degree_to_radian(50);
	camera.aspect = (float)vulkan->swap_chain.image_width / (float)vulkan->swap_chain.image_height;
	camera.znear = 0.1f;
	camera.zfar = 1000;
	return camera;
}

template <typename F>
void traverse_model_node_hierarchy(model_node *nodes, uint32 index, F f) {
	model_node *node = &nodes[index];
	f(node);
	for (uint32 i = 0; i < node->child_count; i += 1) {
		traverse_model_node_hierarchy(nodes, node->children[i], f);
	}
}

template <typename F>
void traverse_model_scenes(model *model, F f) {
	for (uint32 i = 0; i < model->scene_count; i += 1) {
		model_scene *scene = &model->scenes[i];
		for (uint32 i = 0; i < scene->node_index_count; i += 1) {
			traverse_model_node_hierarchy(model->nodes, scene->node_indices[i], f);
		}
	}
}

template <typename F>
void traverse_model_node_hierarchy_track_global_transform(model_node *nodes, uint32 index, mat4 global_transform_mat, F f) {
	model_node *node = &nodes[index];
	global_transform_mat = global_transform_mat * node->local_transform_mat;
	f(node, global_transform_mat);
	for (uint32 i = 0; i < node->child_count; i += 1) {
		traverse_model_node_hierarchy_track_global_transform(nodes, node->children[i], global_transform_mat, f);
	}
}

template <typename F>
void traverse_model_scenes_track_global_transform(model *model, F f) {
	for (uint32 i = 0; i < model->scene_count; i += 1) {
		model_scene *scene = &model->scenes[i];
		for (uint32 i = 0; i < scene->node_index_count; i += 1) {
			traverse_model_node_hierarchy_track_global_transform(model->nodes, scene->node_indices[i], mat4_identity(), f);
		}
	}
}

template <typename F>
void level_generate_render_data(level *level, vulkan *vulkan, camera camera, F generate_extra_render_data) {
	level->render_data = {};
	vulkan->buffers.frame_vertex_buffer_offsets[vulkan->frame_index] = 0;
	vulkan->buffers.frame_uniform_buffer_offsets[vulkan->frame_index] = 0;

	uint32 uniform_alignment = (uint32)vulkan->device.physical_device_properties.limits.minUniformBufferOffsetAlignment;

	ambient_light ambient_light = {{0, 0, 0}};
	directional_light directional_light = {{0, 0, 0}, {1, 0, 0}};
	point_light point_lights[level_max_point_light_count] = {};
	uint32 point_light_count = 0;
	for (uint32 i = 0; i < level->light_component_count; i += 1) {
		switch (level->light_components[i].light_type) {
			case light_type_ambient: {
				ambient_light = level->light_components[i].ambient_light;
			} break;
			case light_type_directional: {
				directional_light = level->light_components[i].directional_light;
			} break;
			case light_type_point: {
				m_assert(point_light_count < m_countof(point_lights));
				point_lights[point_light_count++] = level->light_components[i].point_light;
			} break;
		}
	}
	{ // common uniform
		struct common_uniform {
			mat4 camera_view_proj_mat;
			vec4 camera_position;
			mat4 shadow_map_proj_mat;
			struct {
				vec4 color;
			} ambient_light;
			struct {
				vec4 color;
				vec4 direction;
			} directional_lights[level_max_directional_light_count];
			struct {
				vec4 color;
				vec4 position_attenuation;
			} point_lights[level_max_point_light_count];
			struct {
				vec4 color;
				vec4 position_attenuation;
				vec4 direction_angle;
			} spot_lights[level_max_spot_light_count];
			uint32 directional_light_count;
			uint32 point_light_count;
			uint32 spot_light_count;
		};
		round_up(&vulkan->buffers.frame_uniform_buffer_offsets[vulkan->frame_index], uniform_alignment);
		level->render_data.common_data_frame_uniform_buffer_offset = vulkan->buffers.frame_uniform_buffer_offsets[vulkan->frame_index];
		common_uniform *uniform = (struct common_uniform *)(vulkan->buffers.frame_uniform_buffer_ptrs[vulkan->frame_index] + vulkan->buffers.frame_uniform_buffer_offsets[vulkan->frame_index]);
		uniform->camera_view_proj_mat = mat4_vulkan_clip() * camera_view_projection_mat4(camera);
		uniform->camera_position = vec4{m_unpack3(camera.position.e), 0};
		struct camera shadow_map_camera = camera;
		shadow_map_camera.zfar = 100;
		uniform->shadow_map_proj_mat = mat4_vulkan_clip() * camera_shadow_map_projection_mat4(shadow_map_camera, directional_light.direction);
		uniform->ambient_light = {vec4{ambient_light.color.r, ambient_light.color.g, ambient_light.color.b, 0}};
		uniform->directional_lights[0].color = vec4{directional_light.color.r, directional_light.color.g, directional_light.color.b, 0};
		uniform->directional_lights[0].direction = vec4{directional_light.direction.x, directional_light.direction.y, directional_light.direction.z, 0};
		uniform->directional_light_count = 1;
		for (uint32 i = 0; i < m_countof(point_lights); i += 1) {
			uniform->point_lights[i].color = {point_lights[i].color.r, point_lights[i].color.g, point_lights[i].color.b, 0};
			uniform->point_lights[i].position_attenuation = {point_lights[i].position.x, point_lights[i].position.y, point_lights[i].position.z, point_lights[i].attenuation};
		}
		uniform->point_light_count = point_light_count;
		uniform->spot_light_count = 0;
		vulkan->buffers.frame_uniform_buffer_offsets[vulkan->frame_index] += sizeof(struct common_uniform);
	}
	{ // models
		if (level->render_component_count > 0) {
			round_up(&vulkan->buffers.frame_uniform_buffer_offsets[vulkan->frame_index], uniform_alignment);
			level->render_data.models = allocate_memory<struct model_render_data>(&level->render_thread_frame_memory_arena, level->render_component_count);
			level->render_data.model_count = 0;
			for (uint32 i = 0; i < level->entity_count; i += 1) {
				if (level->entity_flags[i] & entity_component_flag_render) {
					entity_render_component *render_component = entity_get_render_component(level, i);
					if (!render_component->hide && render_component->model_index < level->model_count) {
						model_render_data *model_render_data = &level->render_data.models[level->render_data.model_count++];
						model_render_data->model_index = render_component->model_index;
						model *model = &level->models[render_component->model_index];
						model_render_data->mesh_count = 0;
						traverse_model_scenes(model, [&](model_node *node) {
							if (node->mesh_index < model->mesh_count) {
								model_render_data->mesh_count += 1;
							}
						});
						mat4 transform_mat = transform_to_mat4(level->entity_transforms[i]) * transform_to_mat4(render_component->adjustment_transform);
						model_render_data->meshes = allocate_memory<struct mesh_render_data>(&level->render_thread_frame_memory_arena, model_render_data->mesh_count);
						uint32 mesh_render_data_index = 0;
						traverse_model_scenes_track_global_transform(model, [&](model_node *node, mat4 global_transform) {
							if (node->mesh_index < model->mesh_count) {
								mesh_render_data *mesh = &model_render_data->meshes[mesh_render_data_index++];
								mesh->mesh_index = node->mesh_index;
								struct {
									mat4 model_mat;
									mat4 normal_mat;
									vec2 uv_scale;
									float roughness;
									float metallic;
									float height_map_scale;
								} uniforms = {};
								uniforms.model_mat = transform_mat * global_transform;
								uniforms.normal_mat = mat4_transpose(mat4_inverse(uniforms.model_mat));
								uniforms.uv_scale = {1, 1};
								uniforms.roughness = 1;
								uniforms.metallic = 0;
								uniforms.height_map_scale = 0;
								uint8 *uniform_buffer_ptr = vulkan->buffers.frame_uniform_buffer_ptrs[vulkan->frame_index];
								uint32 *frame_uniform_buffer_offset = &vulkan->buffers.frame_uniform_buffer_offsets[vulkan->frame_index];
								round_up(frame_uniform_buffer_offset, uniform_alignment);
								mesh->frame_uniform_buffer_offset = *frame_uniform_buffer_offset;
								memcpy(uniform_buffer_ptr + *frame_uniform_buffer_offset, &uniforms, sizeof(uniforms));
								*frame_uniform_buffer_offset += sizeof(uniforms);
							}
						});
					}
				}
			}
		}
	}
	generate_extra_render_data();
}

template <typename F0, typename F1>
void level_generate_render_commands(level *level, vulkan *vulkan, camera camera, F0 extra_main_render_pass_render_commands, F1 extra_swap_chain_render_commands) {
	VkCommandBuffer cmd_buffer = vulkan->cmd_buffers.graphic_cmd_buffers[vulkan->frame_index];
	{ // shadow passes
		{
			VkClearValue clear_values[] = {{1, 1, 0, 0}, {1, 0}};
			VkRenderPassBeginInfo render_pass_begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
			render_pass_begin_info.renderPass = vulkan->render_passes.shadow_map_render_passes[0];
			render_pass_begin_info.framebuffer = vulkan->framebuffers.shadow_map_framebuffers[vulkan->frame_index].framebuffers[0];
			render_pass_begin_info.renderArea.offset = {0, 0};
			render_pass_begin_info.renderArea.extent = {vulkan->framebuffers.shadow_map_framebuffers[vulkan->frame_index].width, vulkan->framebuffers.shadow_map_framebuffers[vulkan->frame_index].height};
			render_pass_begin_info.clearValueCount = m_countof(clear_values);
			render_pass_begin_info.pClearValues = clear_values;
			vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
			VkViewport viewport = {0, 0, (float)vulkan->framebuffers.shadow_map_framebuffers[vulkan->frame_index].width, (float)vulkan->framebuffers.shadow_map_framebuffers[vulkan->frame_index].height, 0, 1};
			VkRect2D scissor = {{0, 0}, vulkan->framebuffers.shadow_map_framebuffers[vulkan->frame_index].width, vulkan->framebuffers.shadow_map_framebuffers[vulkan->frame_index].height};
			vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
			vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);
			if (level->render_data.model_count > 0) {
				vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.static_model_shadow_map_pipeline.pipeline);
				VkDeviceSize vertex_buffer_offset = 0;
				vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &vulkan->buffers.level_vertex_buffer.buffer, &vertex_buffer_offset);
				vkCmdBindIndexBuffer(cmd_buffer, vulkan->buffers.level_vertex_buffer.buffer, 0, VK_INDEX_TYPE_UINT16);
				for (uint32 i = 0; i < level->render_data.model_count; i += 1) {
					model_render_data *model_render_data = &level->render_data.models[i];
					model *model = &level->models[model_render_data->model_index];
					for (uint32 i = 0; i < model_render_data->mesh_count; i += 1) {
						model_mesh *mesh = &model->meshes[model_render_data->meshes[i].mesh_index];
						uint32 offsets[3] = {level->render_data.common_data_frame_uniform_buffer_offset, model_render_data->meshes[i].frame_uniform_buffer_offset, 0};
						vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.static_model_shadow_map_pipeline.layout, 0, 1, &(vulkan->descriptors.frame_uniform_buffer_offsets[vulkan->frame_index]), m_countof(offsets), offsets);
						vkCmdDrawIndexed(cmd_buffer, mesh->index_count, 1, mesh->index_buffer_offset / sizeof(uint16), mesh->vertex_buffer_offset / sizeof(struct gpk_model_vertex), 0);
					}
				}
			}
			vkCmdEndRenderPass(cmd_buffer);
		}
		{
			VkClearValue clear_values[] = {{0, 0, 0, 0}};
			VkRenderPassBeginInfo render_pass_begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
			render_pass_begin_info.renderPass = vulkan->render_passes.shadow_map_render_passes[1];
			render_pass_begin_info.framebuffer = vulkan->framebuffers.shadow_map_framebuffers[vulkan->frame_index].framebuffers[1];
			render_pass_begin_info.renderArea.offset = {0, 0};
			render_pass_begin_info.renderArea.extent = {vulkan->framebuffers.shadow_map_framebuffers[vulkan->frame_index].width, vulkan->framebuffers.shadow_map_framebuffers[vulkan->frame_index].height};
			render_pass_begin_info.clearValueCount = m_countof(clear_values);
			render_pass_begin_info.pClearValues = clear_values;
			vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.shadow_map_gaussian_blur_pipelines[0].pipeline);
			vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.shadow_map_gaussian_blur_pipelines[0].layout, 0, 1, &vulkan->descriptors.shadow_map_framebuffer_textures[vulkan->frame_index][0], 0, nullptr);
		  struct {
		  	float x_dir, y_dir;
		  } push_const = {1, 0};
		  vkCmdPushConstants(cmd_buffer, vulkan->pipelines.shadow_map_gaussian_blur_pipelines[0].layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(push_const), &push_const);
			vkCmdDraw(cmd_buffer, 3, 1, 0, 0);
			vkCmdEndRenderPass(cmd_buffer);
		}
		{
			VkClearValue clear_values[] = {{0, 0, 0, 0}};
			VkRenderPassBeginInfo render_pass_begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
			render_pass_begin_info.renderPass = vulkan->render_passes.shadow_map_render_passes[2];
			render_pass_begin_info.framebuffer = vulkan->framebuffers.shadow_map_framebuffers[vulkan->frame_index].framebuffers[2];
			render_pass_begin_info.renderArea.offset = {0, 0};
			render_pass_begin_info.renderArea.extent = {vulkan->framebuffers.shadow_map_framebuffers[vulkan->frame_index].width, vulkan->framebuffers.shadow_map_framebuffers[vulkan->frame_index].height};
			render_pass_begin_info.clearValueCount = m_countof(clear_values);
			render_pass_begin_info.pClearValues = clear_values;
			vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
			vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.shadow_map_gaussian_blur_pipelines[1].pipeline);
			vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.shadow_map_gaussian_blur_pipelines[1].layout, 0, 1, &vulkan->descriptors.shadow_map_framebuffer_textures[vulkan->frame_index][1], 0, nullptr);
		  struct {
		  	float x_dir, y_dir;
		  } push_const = {0, 1};
		  vkCmdPushConstants(cmd_buffer, vulkan->pipelines.shadow_map_gaussian_blur_pipelines[1].layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(push_const), &push_const);
			vkCmdDraw(cmd_buffer, 3, 1, 0, 0);
			vkCmdEndRenderPass(cmd_buffer);
		}
	}
	{ // main render pass
		VkClearValue clear_values[2] = {{0, 0, 0, 1}, {0, 0}};
		VkRenderPassBeginInfo render_pass_begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
		render_pass_begin_info.renderPass = vulkan->render_passes.main_render_pass;
		render_pass_begin_info.framebuffer = vulkan->framebuffers.main_framebuffers[vulkan->frame_index].framebuffer;
		render_pass_begin_info.renderArea.offset = {0, 0};
		render_pass_begin_info.renderArea.extent = {vulkan->framebuffers.main_framebuffers[vulkan->frame_index].width, vulkan->framebuffers.main_framebuffers[vulkan->frame_index].height};
		render_pass_begin_info.clearValueCount = m_countof(clear_values);
		render_pass_begin_info.pClearValues = clear_values;
		vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport = {0, 0, (float)vulkan->framebuffers.main_framebuffers[vulkan->frame_index].width, (float)vulkan->framebuffers.main_framebuffers[vulkan->frame_index].height, 1, 0};
		VkRect2D scissor = {{0, 0}, vulkan->framebuffers.main_framebuffers[vulkan->frame_index].width, vulkan->framebuffers.main_framebuffers[vulkan->frame_index].height};
		vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
		vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);
		if (level->render_data.model_count > 0) {
			vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.static_model_pipeline.pipeline);
			VkDeviceSize vertex_buffer_offset = 0;
			vkCmdBindVertexBuffers(cmd_buffer, 0, 1, &vulkan->buffers.level_vertex_buffer.buffer, &vertex_buffer_offset);
			vkCmdBindIndexBuffer(cmd_buffer, vulkan->buffers.level_vertex_buffer.buffer, 0, VK_INDEX_TYPE_UINT16);
			vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.static_model_pipeline.layout, 2, 1, &vulkan->descriptors.shadow_map_framebuffer_textures[vulkan->frame_index][2], 0, nullptr);
			for (uint32 i = 0; i < level->render_data.model_count; i += 1) {
				model_render_data *model_render_data = &level->render_data.models[i];
				model *model = &level->models[model_render_data->model_index];
				for (uint32 i = 0; i < model_render_data->mesh_count; i += 1) {
					model_mesh *mesh = &model->meshes[model_render_data->meshes[i].mesh_index];
					uint32 frame_uniform_buffer_offsets[3] = {level->render_data.common_data_frame_uniform_buffer_offset, model_render_data->meshes[i].frame_uniform_buffer_offset, 0};
					vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.static_model_pipeline.layout, 0, 1, &(vulkan->descriptors.frame_uniform_buffer_offsets[vulkan->frame_index]), m_countof(frame_uniform_buffer_offsets), frame_uniform_buffer_offsets);
					if (mesh->material_index < model->material_count) {
 						vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.static_model_pipeline.layout, 1, 1, &model->materials[mesh->material_index].textures_descriptor_set, 0, nullptr);
					}
					else {
						vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.static_model_pipeline.layout, 1, 1, &vulkan->descriptors.model_default_material_textures, 0, nullptr);
					}
					vkCmdDrawIndexed(cmd_buffer, mesh->index_count, 1, mesh->index_buffer_offset / sizeof(uint16), mesh->vertex_buffer_offset / sizeof(struct gpk_model_vertex), 0);
					if (model_render_data->meshes[i].render_vertices_outline) {
						vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.static_model_wireframe_pipeline.pipeline);
						uint32 frame_uniform_buffer_offsets[3] = {level->render_data.common_data_frame_uniform_buffer_offset, model_render_data->meshes[i].frame_uniform_buffer_offset, 0};
						vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.static_model_wireframe_pipeline.layout, 0, 1, &(vulkan->descriptors.frame_uniform_buffer_offsets[vulkan->frame_index]), m_countof(frame_uniform_buffer_offsets), frame_uniform_buffer_offsets);
						vkCmdDrawIndexed(cmd_buffer, mesh->index_count, 1, mesh->index_buffer_offset / sizeof(uint16), mesh->vertex_buffer_offset / sizeof(struct gpk_model_vertex), 0);
						vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.static_model_pipeline.pipeline);
					}
				}
			}
		}
		if (level->skybox_index < level->skybox_count) {
		  skybox *skybox = &level->skyboxes[level->skybox_index];
		  vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.skybox_pipeline.pipeline);
		  vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.skybox_pipeline.layout, 0, 1, &skybox->descriptor_set, 0, nullptr);
		  struct camera skybox_camera = camera;
		  skybox_camera.position = {0, 0, 0};
		  struct {
		  	mat4 camera_view_proj_mat;
		  } push_const = {mat4_vulkan_clip() * camera_view_projection_mat4(skybox_camera)};
		  vkCmdPushConstants(cmd_buffer, vulkan->pipelines.skybox_pipeline.layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(push_const), &push_const);
		  vkCmdDraw(cmd_buffer, 36, 1, 0, 0);
		}
		extra_main_render_pass_render_commands();
		vkCmdEndRenderPass(cmd_buffer);
	}
	{ // swap chain render pass
		VkClearValue clear_value = {0, 0, 0, 1};
		VkRenderPassBeginInfo render_pass_begin_info = {VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO};
		render_pass_begin_info.renderPass = vulkan->render_passes.swap_chain_render_pass;
		render_pass_begin_info.framebuffer = vulkan->framebuffers.swap_chain_framebuffers[vulkan->swap_chain_image_index];
		render_pass_begin_info.renderArea.offset = {0, 0};
		render_pass_begin_info.renderArea.extent = {vulkan->swap_chain.image_width, vulkan->swap_chain.image_height};
		render_pass_begin_info.clearValueCount = 1;
		render_pass_begin_info.pClearValues = &clear_value;
		vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);
		VkViewport viewport = {0, 0, (float)vulkan->swap_chain.image_width, (float)vulkan->swap_chain.image_height, 0, 1};
		VkRect2D scissor = {{0, 0}, {vulkan->swap_chain.image_width, vulkan->swap_chain.image_height}};
		vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);
		vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);
		{
			vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.swap_chain_pipeline.pipeline);
			vkCmdBindDescriptorSets(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->pipelines.swap_chain_pipeline.layout, 0, 1, &vulkan->descriptors.main_framebuffer_textures[vulkan->frame_index], 0, nullptr);
			vkCmdDraw(cmd_buffer, 3, 1, 0, 0);
		}
		extra_swap_chain_render_commands();
		vkCmdEndRenderPass(cmd_buffer);
	}
}	

#if 0
{ // texts
	struct vertex {
		vec4 position_uv;
		u8vec4 color;
		uint32 transform_mat_index;
	};
	static_assert(sizeof(struct vertex) == 24, "");
	round_up(&vulkan->buffers.frame_vertex_buffer_offsets[vulkan->frame_index], (uint32)sizeof(struct vertex));
	round_up(&vulkan->buffers.frame_uniform_buffer_offsets[vulkan->frame_index], uniform_alignment);
	level->render_data.text_frame_vertex_buffer_offset = vulkan->buffers.frame_vertex_buffer_offsets[vulkan->frame_index];
	level->render_data.text_frame_uniform_buffer_offset = vulkan->buffers.frame_uniform_buffer_offsets[vulkan->frame_index];
	level->render_data.text_frame_vertex_count = 0;
	level->render_data.text_frame_uniform_count = 0;
	auto append_text_render_data = [level, vulkan, font, camera](const char *text, aa_bound bound) {
		vec4 text_quad = {};
		float xpos = 0;
		float ypos = 0;
		uint32 text_len = (uint32)strlen(text);
		for (uint32 i = 0; i < text_len; i += 1) {
			stbtt_aligned_quad quad = {};
			stbtt_GetPackedQuad(font->stbtt_packed_chars, font->packed_bitmap_width, font->packed_bitmap_height, text[i] - ' ', &xpos, &ypos, &quad, 0);
			quad.y0 = -quad.y0;
			quad.y1 = -quad.y1;
			text_quad.x0 = min(text_quad.x0, quad.x0);
			text_quad.y0 = max(text_quad.y0, quad.y0);
			text_quad.x1 = max(text_quad.x1, quad.x1);
			text_quad.y1 = min(text_quad.y1, quad.y1);
			vertex *vertices = (vertex *)(vulkan->buffers.frame_vertex_buffer_ptrs[vulkan->frame_index] + vulkan->buffers.frame_vertex_buffer_offsets[vulkan->frame_index] + sizeof(struct vertex) * 6 * i);
			vertices[0] = {{quad.x0, quad.y0, quad.s0, quad.t0}, {0, 255, 0, 0}, level->render_data.text_frame_uniform_count};
			vertices[1] = {{quad.x0, quad.y1, quad.s0, quad.t1}, {0, 255, 0, 0}, level->render_data.text_frame_uniform_count};
			vertices[2] = {{quad.x1, quad.y1, quad.s1, quad.t1}, {0, 255, 0, 0}, level->render_data.text_frame_uniform_count};
			vertices[3] = vertices[0];
			vertices[4] = vertices[2];
			vertices[5] = {{quad.x1, quad.y0, quad.s1, quad.t0}, {0, 255, 0, 255}, level->render_data.text_frame_uniform_count};
		}
		vulkan->buffers.frame_vertex_buffer_offsets[vulkan->frame_index] += sizeof(struct vertex) * 6 * text_len;

		vec2 text_quad_center = {};
		text_quad_center.x = text_quad.x0 + (text_quad.x1 - text_quad.x0) / 2;
		text_quad_center.y = text_quad.y0 + (text_quad.y1 - text_quad.y0) / 2;

		float text_quad_scale = 1 / font->font_size;
		float text_quad_height = text_quad.y0 - text_quad.y1;
		vec3 text_quad_translate = {0, text_quad_height / 2 * text_quad_scale, 0};
		text_quad_translate = text_quad_translate + aa_bound_center(bound);
		text_quad_translate.y += (bound.max.y - bound.min.y) / 2;

		mat4 transform_mat = mat4_vulkan_clip() * camera_view_projection_mat4(camera) * mat4_from_translation(text_quad_translate) * camera_billboard_mat4(camera) * mat4_from_scaling(text_quad_scale) * mat4_from_translation({(-text_quad_center).x, (-text_quad_center).y, 0});
		*(mat4 *)(vulkan->buffers.frame_uniform_buffer_ptrs[vulkan->frame_index] + vulkan->buffers.frame_uniform_buffer_offsets[vulkan->frame_index]) = transform_mat;
		vulkan->buffers.frame_uniform_buffer_offsets[vulkan->frame_index] += sizeof(mat4);

		level->render_data.text_frame_vertex_count += text_len * 6;
		level->render_data.text_frame_uniform_count += 1;
	};
	for (uint32 i = 0; i < level->entity_count; i += 1) {
		append_text_render_data(level->entity_infos[i].name, aa_bound{{-1, -1, -1}, {1, 1, 1}});
	}
}
#endif
