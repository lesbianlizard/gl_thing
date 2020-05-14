#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/inotify.h>

#include <jack/jack.h>

// FIXME: another gl-extension-loading library to look into
//#include <epoxy/gl.h>
//#include <epoxy/glx.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <GL/freeglut_ext.h>

#ifndef USE_GETTEXT
  #define _(String) (String)
  #define N_(String) (String)
#endif

#include "c_utils/cstr_utils.h"
#include "utils.h"

typedef jack_default_audio_sample_t jack_sample_t;

jack_port_t *jack_input_port;
jack_port_t *jack_output_port;
jack_client_t *jack_client;
// FIXME: these shouldn't need to be initialized, but haaacks!
jack_nframes_t jack_sample_rate = 48000;
jack_nframes_t jack_buffer_size = 512;


static char* vsh_filename = "vertex.vert";
static char* fsh_filename = "fragment.frag";
static char* gsh_filename = "put_cubes.geom";

static GLint win = 0;
volatile int colormode = 0;
volatile int should_recompile = 0;

GLuint
  VAO,
  VBO,
  EBO,
  vertexShader,
  fragmentShader,
  geometryShader,
  shaderProgram,
  offset_texture,
  texture_2d;
GLfloat color_anim = 0;
GLfloat color_step = 0.001;
GLfloat color_dir = 1;
GLenum error;
GLsizei n_things_to_draw;

// This is the shared buffer between the geometry shader and jack
GLsizei offset_tex_len = 1920;
GLfloat *offset_tex_data;
GLuint graph_period = 10; // seconds

jack_sample_t *jack_raw_buffer;
// The next position that should be written to in jack_raw_buffer, in multiples of jack_buffer_size
size_t jack_raw_buf_pos = 0; 
// FIXME: confusing name, how to distinguish between lengh as in elements and as in bytes?
size_t jack_raw_buf_bytes;

pthread_t threads[2];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

#include "jack_simple_client.h"

// FIXME: function headers for everyone
void recompile_shaders(void);

static void draw_callback(void)
{
  //printf("[%s] starting, colormode = %i\n", __func__, colormode);
  
  // If the recompile thread has set the flag, then recompile shaders
  if (should_recompile == 1)
  {
    recompile_shaders();
    should_recompile = 0;
  }

  // update offset texture
  jack_buffer_to_offset_tex(jack_raw_buffer, jack_raw_buf_bytes, offset_tex_data, offset_tex_len);

  // Animate background color
  if (!((color_anim + color_dir*color_step > 0) && (color_anim + color_dir*color_step < 0.2)))
  {
    color_dir *= -1;
    //printf("[%s] color_dir = %f\n", __func__, color_dir);
  }

  color_anim += color_dir*color_step; 
  //printf("[%s] color_anim = %f\n", __func__, color_anim);

  glClearColor(color_anim, 0.0f, 0.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);

  
  //glDrawArrays(GL_TRIANGLES, 0, 3);
  glDrawArrays(GL_POINTS, 0, n_things_to_draw);
  //glDrawElements(GL_POINTS, 6, GL_UNSIGNED_INT, 0);
    
  glutSwapBuffers();

  error = glGetError();
  if (!(error == 0))
  {
  printf("[%1$s] While rendering, glGetError reports error %2$i.\n",
    __func__,
    error);
  }
}

// Scale down the buffer of jack data to fit inside the screen-space offset texture
void
jack_buffer_to_offset_tex(jack_sample_t *jack_buf, size_t jack_buf_bytes, GLfloat *offset_tex, size_t offset_tex_len)
{
  // FIXME: come up with an algorithm which finds exactly the right indices, not using integer division...
  size_t
    ratio,
    i,
    idx,
    jack_raw_buf_pos_local = jack_raw_buf_pos;

  ratio = jack_buf_bytes/(sizeof(GLfloat) * offset_tex_len);
  printf(_("[%1$s] scaling with ratio %2$li\n"), __func__, ratio);

  for (i = 0; i < offset_tex_len; i++)
  {
    idx = (jack_raw_buf_pos_local*jack_buffer_size + ratio * i) % jack_buf_bytes;
    printf(_("[%1$s] copying value %2$f at jack index %2$li (texture index %3$li) into offset texture.\n"), __func__, jack_buf[idx], idx, i);

    offset_tex[i] = jack_buf[idx];
  }
}

int
shader_compile(char *shader_filename, unsigned int *shader_ret, GLenum shaderType)
{
  int fd;
  // FIXME: make dynamic
  char compile_log[512];
  GLchar *shader_source;
  GLuint new_shader;
  GLint success;

  // Memory map shader source code into C string
  if (mmap_file_cstr(shader_filename, &fd, &shader_source) < 0)
  {
    return -1;
  }

  // Compile vertex shader
  new_shader = glCreateShader(shaderType);
  glShaderSource(new_shader, 1, (const GLchar**) &shader_source, NULL);
  glCompileShader(new_shader);

  // Check if vertex shader is ok
  glGetShaderiv(new_shader, GL_COMPILE_STATUS, &success);

  if (!success)
  {
    glGetShaderInfoLog(new_shader, 512, NULL, compile_log);
    printf(_("[%1$s] Compiling shader '%2$s' of type %3$i failed with status %4$i, log:\n%5$s"),
      __func__,
      shader_filename,
      shaderType,
      success,
      compile_log);
    return -1;
  }

    printf(_("[%1$s] Successfully compiled shader '%2$s' of type %3$i\n"),
      __func__,
      shader_filename,
      shaderType);
  close(fd);
  *shader_ret = new_shader;
  return 0;
}

int
shader_link(GLuint vsh, GLuint fsh, GLuint gsh, GLuint *prog_ret)
{
  GLuint prog;
  GLint success;
  // FIXME: make this dynamic
  char compile_log[512];

  // Link program
  prog = glCreateProgram();
  glAttachShader(prog, vsh);
  glAttachShader(prog, fsh);
  glAttachShader(prog, gsh);
  glLinkProgram(prog);

  // Check if program link is ok 
  glGetProgramiv(prog, GL_LINK_STATUS, &success);
  if (!success)
  {
  glGetProgramInfoLog(prog, 512, NULL, compile_log);
    printf("[%s] Program link failed with status %i, log:\n%s",
      __func__,
      success,
      compile_log);
    return -1;
  }

  // We don't need these anymore
  glDeleteShader(vsh);
  glDeleteShader(fsh);
  glDeleteShader(gsh);
  printf("[%s] Program link succeeded\n",
    __func__);
  *prog_ret = prog;
  return 0;
}

void
recompile_shaders(void)
{
  printf(_("[%1$s] begin\n"), __func__);

  // Compile shaders
  if (
      shader_compile(vsh_filename, &vertexShader, GL_VERTEX_SHADER) > -1 &&
      shader_compile(fsh_filename, &fragmentShader, GL_FRAGMENT_SHADER) > -1 &&
      shader_compile(gsh_filename, &geometryShader, GL_GEOMETRY_SHADER) > -1
     )
  {
    // Link shader program
    if (shader_link(vertexShader, fragmentShader, geometryShader, &shaderProgram) > -1)
    {
      printf(_("[%1$s] Successfully linked program, using it\n"), __func__);
      // And use it
      glUseProgram(shaderProgram);
    }
    else
    {
      printf(_("[%1$s] Linking error, not using program\n"), __func__);
    }
  }
}

void*
check_recompile_thread(void *mutex)
{
  // FIXME: add error checking to this function
  int
    inotify_fd,
    w1 = 0,
    w2 = 0,
    w3 = 0;
  struct inotify_event
    event;
  
  while (1)
  {
    inotify_fd = inotify_init();

    // make sure we get a valid watch on both files before continuing
    do
    {
      // FIXME: maybe should check for more masks
      w1 = inotify_add_watch(inotify_fd, vsh_filename, IN_MODIFY | IN_MOVE_SELF);
      w2 = inotify_add_watch(inotify_fd, fsh_filename, IN_MODIFY | IN_MOVE_SELF);
      w3 = inotify_add_watch(inotify_fd, gsh_filename, IN_MODIFY | IN_MOVE_SELF);
      printf("[%1$s] inotify watch fds added %2$s: %3$i, %4$s: %5$i, %6$s: %7$i\n",
        __func__,
        vsh_filename, w1,
        fsh_filename, w2,
        gsh_filename, w3);
    }
    while (w1 < 0 || w2 < 0 || w3 < 0);
    
    read(inotify_fd, &event, sizeof(struct inotify_event));
    printf("[%1$s] A shader source file changed with mask %2$i, recompiling\n",
      __func__,
      event.mask);

    // FIXME: do re really have to close and reinit the inotify every time?
    close(inotify_fd);
    should_recompile = 1;
  }
}




int main(int argc, char **argv)
{
  printf("[%1$s] begin\n", __func__);
  // FIXME: properly init window size with vars and stuff
  //glutInitWindowSize(800, 600);
  glutInit(&argc, argv);
  printf("[%1$s] glutInit done\n", __func__);
  //glutInitContextFlags(0x00022007);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  printf("[%1$s] glutInitDisplayMode done\n", __func__);
  win = glutCreateWindow("sort of a glxgears clone");
  printf("[%1$s] glutCreateWindow done\n", __func__);
  //glViewport(0, 0, 800, 600);
  
  glewInit();
  printf("[%1$s] glewInit done\n", __func__);

  printf("GL_VERSION  : %s\n", glGetString(GL_VERSION) );
  printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER) );

  // Compile shaders at least once
  recompile_shaders();

  // Start shader recompile thread
  pthread_create(&threads[0], NULL, check_recompile_thread, &mutex);

  // A very simple 1D texture
//  GLfloat offset_tex_data[] = {
//    0.0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9
//  };

  GLsizei n_verts;
  GLfloat *vertices;
  GLuint n_dims, i, j;
  GLfloat temp;
  
  offset_tex_data = malloc(offset_tex_len * sizeof(GLfloat));

  for (j = 0; j < offset_tex_len; j++)
  {
    // FIXME: get the endpoints exactly right
    temp = ((GLfloat) j) / offset_tex_len;
    offset_tex_data[j] = temp;
  }

  n_dims = 3;
  n_verts = 1920;
  vertices = malloc(n_verts * n_dims * sizeof(GLfloat));
  //memset(vertices, 0, n_verts * sizeof(GLFloat));

  for (i = 0; i < n_dims; i++)
  {
    for (j = 0 + i; j < n_dims*(n_verts - 1) + i + 1; j += n_dims)
    {
      // x-position
      if (i == 0)
      {
        // FIXME: get the endpoints exactly right
        temp = ((GLfloat) j) / n_dims / n_verts * 2 - 1;
      }
      // y-position
      else if (i == 1)
      {
        temp = -0.5;
      }
      // z-position
      else
      {
        temp = 0;
      }

      vertices[j] = temp;
    }
  }

  n_things_to_draw = n_verts;

//  GLuint indices[] = {
//    0, 1, 2,
//    0, 3, 2,
//  };

  // Lock mutex to ensure jack doesn't write to jack_raw_buffer before we init it below
  // FIXME: but the thread needs to run first to generate the constants below, add more mutexes!
  pthread_mutex_lock(&mutex);
  // Start jack client after offset texture buffer has been allocated
  pthread_create(&threads[1], NULL, jack_main, &mutex);

  jack_raw_buf_bytes = round_up_integer(jack_sample_rate * graph_period * sizeof(jack_sample_t), jack_buffer_size);
  printf("[%s] allocating %li bytes (%li * jack_buffer_size) for jack_raw_buffer\n", __func__, jack_raw_buf_bytes, jack_raw_buf_bytes/jack_buffer_size);
  jack_raw_buffer = malloc(jack_raw_buf_bytes);
  pthread_mutex_unlock(&mutex);

  //glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  //glGenBuffers(1, &EBO);

  //glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, n_dims * n_verts * sizeof(GLfloat), vertices, GL_STATIC_DRAW);

  //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*) 0);
  glEnableVertexAttribArray(0);

  //glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*) (3 * sizeof(GLfloat)));
  //glEnableVertexAttribArray(1);

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // set up the texture thing
  glGenTextures(1, &offset_texture);
  glBindTexture(GL_TEXTURE_1D, offset_texture);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RED, offset_tex_len, 0, GL_RED, GL_FLOAT, offset_tex_data);
  // Disallow graphing "out of bounds"
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // This is important, the texture doesn't seem to work without it!
  glGenerateMipmap(GL_TEXTURE_1D);
  
  // set up the texture thing
//  glGenTextures(1, &texture_2d);
//  glBindTexture(GL_TEXTURE_2D, offset_texture);
//  glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 10, 10, 0, GL_RED, GL_FLOAT, tex_2d_data);
//  glGenerateMipmap(GL_TEXTURE_2D);

  printf("[%1$s] Before starting rendering, glGetError reports error %2$i.\n",
    __func__,
    glGetError());

  glutDisplayFunc(draw_callback);
  glutIdleFunc(glutPostRedisplay);
  glutMainLoop();

  // FIXME: an interrupt handler that actually ever runs these
  free(offset_tex_data);
  free(vertices);
  return 0;
}
