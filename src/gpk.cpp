/***************************************************************************************************/
/*          Copyright (C) 2017-2018 By Yang Chen (yngccc@gmail.com). All Rights Reserved.          */
/***************************************************************************************************/

#ifndef __GPK_CPP__
#define __GPK_CPP__

#define m_gpk_model_format_str "GPK_MODEL_FORMAT"
#define m_gpk_skybox_format_str "GPK_SKYBOX_FORMAT"
#define m_gpk_terrain_format_str "GPK_TERRAIN_FORMAT"

struct gpk_model {
	char format_str[32];
	uint32 scene_offset;
	uint32 scene_count;
	uint32 node_offset;
	uint32 node_count;
	uint32 mesh_offset;
	uint32 mesh_count;
	uint32 skin_offset;
	uint32 skin_count;
	uint32 animation_offset;
	uint32 animation_count;
	uint32 material_offset;
	uint32 material_count;
	uint32 image_offset;
	uint32 image_count;
};

struct gpk_model_scene {
	char name[64];
	uint32 node_indices[32];
	uint32 node_index_count;
};

struct gpk_model_node {
	uint32 mesh_index;
	uint32 skin_index;
	transform local_transform;
	mat4 local_transform_mat;
	mat4 global_transform_mat;
	uint32 children[128];
	uint32 child_count;
};

struct gpk_model_mesh {
	char name[64];
	uint32 skin_index;
	uint32 primitive_offset;
	uint32 primitive_count;
};

struct gpk_model_mesh_primitive {
	uint32 material_index;
	uint32 indices_offset;
	uint32 index_count;
	uint32 vertices_offset;
	uint32 vertex_count;
};

struct gpk_model_vertex {
	vec3 position;
	u8vec4 color;
	vec2 uv;
	i16vec4 normal;
	i16vec4 tangent;
	u8vec4 joints;
	u16vec4 weights;
};
static_assert(sizeof(gpk_model_vertex) == 12 + 4 + 8 + 8 + 8 + 4 + 8, "");

struct gpk_model_skin {
	char name[64];
	uint32 joint_count;
	uint32 joints_offset;
};

struct gpk_model_joint {
	uint32 node_index;
	mat4 inverse_bind_mat;
};

struct gpk_model_animation {
	char name[64];
	uint32 channel_offset;
	uint32 channel_count;
	uint32 sampler_offset;
	uint32 sampler_count;
};

const uint32 gpk_model_animation_translate_channel = 0;
const uint32 gpk_model_animation_rotate_channel = 1;
const uint32 gpk_model_animation_scale_channel = 2;
const uint32 gpk_model_animation_weights_channel = 3;

struct gpk_model_animation_channel {
	uint32 node_index;
	uint32 channel_type;
	uint32 sampler_index;
};

const uint32 gpk_model_animation_linear_interpolation = 0;
const uint32 gpk_model_animation_step_interpolation = 1;
const uint32 gpk_model_animation_catmullromspline_interpolation = 2;
const uint32 gpk_model_animation_cubicspline_interpolation = 3;

struct gpk_model_animation_sampler {
	uint32 interpolation_type;
	uint32 key_frame_count;
	uint32 key_frame_offset;
};

struct gpk_model_animation_key_frame {
	float time;
	vec4 transform_data;
};

struct gpk_model_material {
	char name[64];
	uint32 diffuse_image_index;
	vec4 diffuse_factor;
	uint32 normal_image_index;
	uint32 roughness_image_index;
	float roughness_factor;
	uint32 metallic_image_index;
	float metallic_factor;
	uint32 emissive_image_index;
	vec3 emissive_factor;
	float transparency;
	float index_of_refraction;
};

struct gpk_model_image {
	uint32 width;
	uint32 height;
	uint32 mips;
	uint32 layer_count;
	uint32 size;
	uint32 format;
	uint32 data_offset;
};

struct gpk_skybox {
	char format_str[32];
	uint32 cubemap_offset;
	uint32 cubemap_width;
	uint32 cubemap_height;
	uint32 cubemap_mipmap_count;
	uint32 cubemap_layer_count;
	uint32 cubemap_size;
	uint32 cubemap_format;
	uint32 cubemap_format_block_dimension;
	uint32 cubemap_format_block_size;
};

struct gpk_terrain {
	char format_str[32];
	uint32 total_size;
	uint32 width;
	uint32 height;
	float max_height;
	uint32 sample_per_meter;
	uint32 height_map_offset;
	uint32 diffuse_map_offset;
};

#endif // __GPK_CPP__
