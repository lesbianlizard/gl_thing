#version 330 core
layout (points) in;
//layout (points, max_vertices = 4) out;
layout (triangle_strip, max_vertices = 3) out;

uniform sampler1D offset_tex;

//FragColor = texture(tex_in, tex_coord_shared).rrra;

void main()
{
  vec4 center;
  float square_size = 0.02;
  int i, j;
  
  // The original position
  gl_Position = gl_in[0].gl_Position;
  EmitVertex();


  center = gl_Position + vec4(0.0, texture(offset_tex, (gl_Position.x + 1)/2).r, 0.0, 0.0);

  gl_Position = center + vec4(square_size*-1.0, square_size*-1.0, 0.0, 0.0);
  EmitVertex();
  gl_Position = center + vec4(square_size*1.0, square_size*1.0, 0.0, 0.0);
  EmitVertex();
  gl_Position = center + vec4(square_size*-1.0, square_size*1.0, 0.0, 0.0);
  EmitVertex();
  //gl_Position = center + vec4(square_size*1.0, square_size*-1.0, 0.0, 0.0);
  //EmitVertex();


  

  EndPrimitive();
}
