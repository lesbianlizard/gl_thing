#version 330 core
out vec4 FragColor;

//in float tex_coord_shared;
//in vec4 color;

//uniform sampler1D tex_in;

void main()
{
  FragColor = vec4(0.0f, 0.65f, 1.0f, 1.0f);
  //FragColor = color;
  //FragColor = texture(tex_in, tex_coord_shared).rrra;
  //FragColor = texture(tex_in, vec2(0.0, 0.0)).rgba;
  //FragColor = vec4(tex_coord_shared.x, tex_coord_shared.y, 0, 1);
}
