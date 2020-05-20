#version 330 core
layout (location = 0) in vec3 pos;

uniform sampler1D offset_tex;

void main()
{
  vec4 vertex_pos_orig = vec4(pos.x, pos.y, pos.z, 1.0);

  gl_Position = vertex_pos_orig + vec4(0.0, 0.5 + 2*(texture(offset_tex, (vertex_pos_orig.x + 1)/2) - 0.25).r, 0.0, 0.0);
}
