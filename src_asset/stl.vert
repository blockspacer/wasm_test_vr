#version 300 es

in vec4 vec4_position;
uniform mat4 mat4_view;
uniform mat4 mat4_projection;

void main() {
    gl_Position = mat4_projection * mat4_view * vec4_position;
}
