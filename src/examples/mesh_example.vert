uniform VertInfo {
  mat4 mvp;
}
vert_info;

in vec3 position;
in vec3 normal;
in vec3 color;

out vec3 v_normal;
out vec3 v_color;

void main() {
  gl_Position = vert_info.mvp * vec4(position, 1.0);
  v_normal = normal;
  v_color = color;
}
