#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/inotify.h>

// FIXME: another gl-extension-loading library to look into
//#include <epoxy/gl.h>
//#include <epoxy/glx.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#ifndef USE_GETTEXT
  #define _(String) (String)
  #define N_(String) (String)
#endif

#include "c_utils/cstr_utils.h"

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
  shaderProgram;
GLfloat color_anim = 1;
GLfloat color_step = 0.01;
GLfloat color_dir = 1;

pthread_t threads[1];
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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

  // Animate background color
  if (!((color_anim + color_dir*color_step > 0) && (color_anim + color_dir*color_step < 1)))
  {
    color_dir *= -1;
    //printf("[%s] color_dir = %f\n", __func__, color_dir);
  }

  color_anim += color_dir*color_step; 
  //printf("[%s] color_anim = %f\n", __func__, color_anim);

  glClearColor(color_anim, 0.0f, 0.0f, 1.0f);

  glClear(GL_COLOR_BUFFER_BIT);
  //glDrawArrays(GL_TRIANGLES, 0, 3);
  glDrawElements(GL_POINTS, 6, GL_UNSIGNED_INT, 0);
    
  glutSwapBuffers();
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
      // And use it
      glUseProgram(shaderProgram);
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
    w2 = 0;
  struct inotify_event
    event;
  
  while (1)
  {
    should_recompile = 1;
    inotify_fd = inotify_init();

    // make sure we get a valid watch on both files before continuing
    do
    {
      // FIXME: maybe should check for more masks
      w1 = inotify_add_watch(inotify_fd, vsh_filename, IN_MODIFY | IN_MOVE_SELF);
      w2 = inotify_add_watch(inotify_fd, fsh_filename, IN_MODIFY | IN_MOVE_SELF);
      printf("[%1$s] inotify watch fds added %2$s: %3$i, %4$s: %5$i\n",
        __func__,
        vsh_filename, w1,
        fsh_filename, w2);
    }
    while (w1 < 0 || w2 < 0);
    
    read(inotify_fd, &event, sizeof(struct inotify_event));
    printf("[%1$s] A shader source file changed with mask %2$i, recompiling\n",
      __func__,
      event.mask);

    // FIXME: do re really have to close and reinit the inotify every time?
    close(inotify_fd);
  }
}




int main(int argc, char **argv)
{
  // FIXME: properly init window size with vars and stuff
  //glutInitWindowSize(800, 600);
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  win = glutCreateWindow("sort of a glxgears clone");
  //glViewport(0, 0, 800, 600);
  
  glewInit();

  printf("GL_VERSION  : %s\n", glGetString(GL_VERSION) );
  printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER) );

  // Start shader recompile thread
  pthread_create(&threads[0], NULL, check_recompile_thread, &mutex);

  // vertices for a square
  GLfloat vertices[] = {
    -1.0f, -1.0f, -1.0f, // bottom left
    1.0f, -1.0f, -1.0f,  // bottom right
    0.8f,  0.8f, 1.0f,  // top right
    -1.0f, 1.0f, -1.0f  // top left
  };  

  GLuint indices[] = {
    0, 1, 2,
    0, 3, 2,
  };

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*) 0);
  glEnableVertexAttribArray(0);

  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glutDisplayFunc(draw_callback);
  glutIdleFunc(glutPostRedisplay);
  glutMainLoop();

  return 0;
}
