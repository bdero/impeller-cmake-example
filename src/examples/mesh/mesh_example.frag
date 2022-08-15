uniform sampler2D base_color_texture;
uniform sampler2D normal_texture;
uniform sampler2D occlusion_roughness_metallic_texture;

in vec3 v_normal;
in vec2 v_texture_coords;

out vec4 frag_color;

void main() {
  frag_color = texture(base_color_texture, v_texture_coords);
  frag_color = texture(normal_texture, v_texture_coords);
  frag_color = texture(occlusion_roughness_metallic_texture, v_texture_coords);
}
