#version 450

#include "common.glsl"

layout(location = 0) in vec3 position_in;
layout(location = 1) in vec2 uv_in;
layout(location = 2) in vec3 normal_in;
layout(location = 3) in vec3 tangent_in;

layout(location = 0) out vec3 position_out;
layout(location = 1) out vec4 shadow_map_coord_out;
layout(location = 2) out vec2 uv_out;
layout(location = 3) out vec3 tbn_position_out;
layout(location = 4) out vec3 tbn_camera_position_out;
layout(location = 5) out vec3 tbn_directional_lights_out[max_directional_light_count];
layout(location = 6) out vec3 tbn_point_lights_out[max_point_light_count];

out gl_PerVertex {
  vec4 gl_Position;
};

layout(set = 0, binding = 0) uniform common_uniform {
  mat4 camera_view_proj_mat;
  vec4 camera_position;
  mat4 shadow_map_proj_mat;
  ambient_light_t ambient_light;
  directional_light_t directional_lights[max_directional_light_count];
  point_light_t point_lights[max_point_light_count];
  spot_light_t spot_lights[max_spot_light_count];
  uint directional_light_count;
  uint point_light_count;
  uint spot_light_count;
};

layout(set = 0, binding = 1) uniform mesh_uniform {
  mat4 model_mat;
  mat4 normal_mat;
  vec2 uv_scale;
  float roughness;
  float metallic;
  float height_map_scale;
};

void main() {
  vec4 position = model_mat * vec4(position_in, 1);
  vec3 normal = normalize(vec3(normal_mat * vec4(normal_in, 0)));
  vec3 tangent = normalize(vec3(normal_mat * vec4(tangent_in, 0)));
  vec3 bitangent = cross(normal, tangent);
  mat3 inverse_tbn_mat = transpose(mat3(tangent, bitangent, normal));

  gl_Position = camera_view_proj_mat * position;
  position_out = position.xyz;
  shadow_map_coord_out = shadow_map_proj_mat * position;
  uv_out = uv_in;
  tbn_position_out = inverse_tbn_mat * position.xyz;
  tbn_camera_position_out = inverse_tbn_mat * camera_position.xyz;
  for (uint i = 0; i < directional_light_count; i += 1) {
    tbn_directional_lights_out[i] = inverse_tbn_mat * directional_lights[i].direction.xyz;
  }
  for (uint i = 0; i < point_light_count; i += 1) {
    tbn_point_lights_out[i] = inverse_tbn_mat * point_lights[i].position.xyz;
  }
} 