#version 330 core
layout (location = 0) in vec3 pos;

//out vec4 color;

void main()
{
  gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);
  //color = (gl_Position.xyzw + 1)/2;
}
