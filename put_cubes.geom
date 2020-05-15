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
  // FIXME: this scaling is kind of ridiculous, fix it. See also jack_buffer_to_offset_tex
  center = gl_Position + vec4(0.0, 0.5 + 2*(texture(offset_tex, (gl_Position.x + 1)/2) - 0.25).r, 0.0, 0.0);


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
