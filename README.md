## gl_thing
This is a small audio visualization program I wrote to teach myself how to use the OpenGL and JACK APIs. The nice tutorial [Learn OpenGL](https://learnopengl.com/) was very helpful, as was [Khronos's OpenGL wiki](https://www.khronos.org/opengl/wiki/). The JACK code was adapted from [simple_client.c](https://github.com/jackaudio/example-clients/blob/master/simple_client.c) from the JACK project's example clients repository.

## Functionality
gl_thing continuously copies a few frames of raw audio data from JACK into a buffer. Every time OpenGL renders a frame, it downsamples the data from the JACK buffer into a 1-dimensional OpenGL texture. The vertex shader in `vertex.vert` uses this texture to offset a highly subdivided line of vertices, effectively drawing an oscilloscope-like graph of the audio waveform on the screen.

## Compiling
You need to install the [freeglut](http://freeglut.sourceforge.net/) and [JACK audio connection kit](https://jackaudio.org/) headers. I've tested this using libraries from the Arch Linux and Raspbian repositories.

Simply run `make` to compile it.

## Running
Run `./thing`. You might need to use a JACK patchbay to feed it some audio data.

It doesn't have any runtime configuration yet, so have a look in `thing.c` if you want to change how the graph looks.

If you edit the shader source files, it will attempt to recompile them on-the-fly.
