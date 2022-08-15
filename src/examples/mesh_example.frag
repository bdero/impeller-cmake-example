in vec3 v_normal;
in vec2 v_texture_coords;

out vec4 frag_color;

void main() {
  frag_color = vec4(v_texture_coords, 0.0, 1.0);
}
