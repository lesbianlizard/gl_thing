#version 330 core
layout (points) in;
layout (points, max_vertices = 3) out;

uniform sampler1D offset_tex;

void main()
{
  gl_Position = gl_in[0].gl_Position;
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(0, texture(offset_tex, 0.1).w, 0, 0);
  EmitVertex();

  gl_Position = gl_in[0].gl_Position + vec4(0, texture(offset_tex, 0.2).w, 0, 0);
  EmitVertex();

  EndPrimitive();
}
