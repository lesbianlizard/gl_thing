#version 330 core
layout (points) in;
layout (points, max_vertices = 1) out;
//layout (triangle_strip, max_vertices = 5) out;

uniform sampler1D offset_tex;

//FragColor = texture(tex_in, tex_coord_shared).rrra;

void main()
{
  vec4 center;
  float square_size = 0.001;
  int i, j;
  
  
  gl_Position = gl_in[0].gl_Position;
  center = gl_Position + vec4(0.0, 0 + 1*texture(offset_tex, (gl_Position.x + 1)/2).r, 0.0, 0.0);


  gl_Position = center + vec4(square_size*-1.0, square_size*-1.0, 0.0, 0.0);
  EmitVertex();
//  gl_Position = center + vec4(square_size*-1.0, square_size*1.0, 0.0, 0.0);
//  EmitVertex();
//  gl_Position = center + vec4(square_size*1.0, square_size*1.0, 0.0, 0.0);
//  EmitVertex();
//  gl_Position = center + vec4(square_size*-1.0, square_size*-1.0, 0.0, 0.0);
//  EmitVertex();
//  gl_Position = center + vec4(square_size*1.0, square_size*-1.0, 0.0, 0.0);
//  EmitVertex();


  

  EndPrimitive();
}
