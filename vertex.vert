#version 330 core
layout (location = 0) in vec3 pos;
layout (location = 1) in vec2 tex_coord;

//out vec4 color;
out vec2 tex_coord_shared;

void main()
{
  gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
  //color = (gl_Position.xyzw + 1)/2;
  tex_coord_shared = tex_coord;
}
