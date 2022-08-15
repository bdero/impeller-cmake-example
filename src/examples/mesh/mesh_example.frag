uniform sampler2D base_color_texture;
uniform sampler2D normal_texture;
uniform sampler2D occlusion_roughness_metallic_texture;

in mat3 v_tangent_space;
in vec2 v_texture_coords;

out vec4 frag_color;

const vec3 kLightDirection = normalize(vec3(0, 1, -2));
const float kGamma = 2.2;

// Convert from sRGB to linear.
// This can be removed once Impeller supports sRGB texture inputs.
vec3 SampleLinear(sampler2D tex, vec2 uv) {
  vec3 color = texture(tex, uv).rgb;
  return pow(color, vec3(kGamma));
}

void main() {
  vec3 albedo = SampleLinear(base_color_texture, v_texture_coords);

  vec3 normal = v_tangent_space *
                (texture(normal_texture, v_texture_coords).rgb * 2.0 - 1.0);
  vec3 orm =
      texture(occlusion_roughness_metallic_texture, v_texture_coords).rgb;
  float occlusion = orm.r;
  float roughness = orm.g;
  float metallic = orm.b;

  frag_color =
      vec4(albedo * 2.0 * occlusion * dot(normal, kLightDirection), 1.0);

#ifndef IMPELLER_TARGET_METAL
  // The Metal framebuffer is already stored in sRGB.
  frag_color.rgb = pow(frag_color.rgb, vec3(1.0 / kGamma));
#endif
}
