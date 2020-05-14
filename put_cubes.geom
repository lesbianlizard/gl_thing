#version 330 core
layout (points) in;
layout (points, max_vertices = 2) out;

uniform sampler1D offset_tex;

//FragColor = texture(tex_in, tex_coord_shared).rrra;

void main()
{
  gl_Position = gl_in[0].gl_Position;
  EmitVertex();

  gl_Position = gl_Position + vec4(0.0, texture(offset_tex, gl_Position.x).r, 0.0, 0.0);
  EmitVertex();

  EndPrimitive();
}
