uniform VertInfo {
  mat4 mvp;
}
vert_info;

in vec3 position;
in vec3 normal;
in vec3 tangent;
in vec2 texture_coords;

out mat3 v_tangent_space;
out vec2 v_texture_coords;

void main() {
  gl_Position = vert_info.mvp * vec4(position, 1.0);
  v_tangent_space =
      mat3(vert_info.mvp) * mat3(tangent, cross(normal, tangent), normal);
  v_texture_coords = texture_coords;
}
