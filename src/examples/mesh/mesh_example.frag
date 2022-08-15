uniform sampler2D base_color_texture;
uniform sampler2D normal_texture;
uniform sampler2D occlusion_roughness_metallic_texture;

in mat3 v_tangent_space;
in vec2 v_texture_coords;

out vec4 frag_color;

const vec3 kLightDirection = normalize(vec3(0, 1, -2));

void main() {
  vec4 albedo = texture(base_color_texture, v_texture_coords);
  vec3 normal = v_tangent_space *
                (texture(normal_texture, v_texture_coords).rgb * 2.0 - 1.0);
  vec3 orm =
      texture(occlusion_roughness_metallic_texture, v_texture_coords).rgb;
  float occlusion = orm.r;
  float roughness = orm.g;
  float metallic = orm.b;

  frag_color =
      vec4(albedo.rgb * 3.0 * occlusion * dot(normal, kLightDirection), 1.0);
}
