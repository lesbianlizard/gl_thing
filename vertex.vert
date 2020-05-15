#version 330 core
layout (location = 0) in vec3 pos;
//layout (location = 1) in float tex_coord;

//out vec4 color;
//out float tex_coord_shared;

uniform sampler1D offset_tex;

void main()
{
  vec4 vertex_pos_orig = vec4(pos.x, pos.y, pos.z, 1.0);

  gl_Position = vertex_pos_orig + vec4(0.0, 0.5 + 2*(texture(offset_tex, (vertex_pos_orig.x + 1)/2) - 0.25).r, 0.0, 0.0);
  //gl_Position = vertex_pos_orig + vec4(0.0, texture(offset_tex, vertex_pos_orig.x).r, 0.0, 0.0);
  //gl_Position = vertex_pos_orig;

  //color = (gl_Position.xyzw + 1)/2;
  //tex_coord_shared = tex_coord;
}
