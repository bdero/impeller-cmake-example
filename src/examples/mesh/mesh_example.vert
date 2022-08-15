uniform VertInfo {
  mat4 mvp;
}
vert_info;

in vec3 position;
in vec3 normal;
in vec2 texture_coords;

out vec3 v_normal;
out vec2 v_texture_coords;

void main() {
  gl_Position = vert_info.mvp * vec4(position, 1.0);
  v_normal = normal;
  v_texture_coords = texture_coords;
}
