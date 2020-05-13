#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <GL/glew.h>
#include <GL/freeglut.h>

#ifndef USE_GETTEXT
  #define _(String) (String)
  #define N_(String) (String)
#endif

#include "c_utils/cstr_utils.h"

static GLint win = 0;
volatile int colormode = 0;

GLuint
  VAO,
  VBO,
  vertexShader,
  fragmentShader,
  shaderProgram;
GLfloat color_anim = 1;
GLfloat color_step = 0.01;
GLfloat color_dir = 1;

static void draw_callback(void)
{
  //printf("[%s] starting, colormode = %i\n", __func__, colormode);

  if (!((color_anim + color_dir*color_step > 0) && (color_anim + color_dir*color_step < 1)))
  {
    color_dir *= -1;
    //printf("[%s] color_dir = %f\n", __func__, color_dir);
  }

  color_anim += color_dir*color_step; 
  //printf("[%s] color_anim = %f\n", __func__, color_anim);

  glClearColor(color_anim, 0.0f, 0.0f, 1.0f);

  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 3);
    
  glutSwapBuffers();
}

int
shader_compile(char *shader_filename, unsigned int *vsh_ret, GLenum shaderType)
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
  *vsh_ret = new_shader;
  return 0;
}

int
shader_link(GLuint vsh, GLuint fsh, GLuint *prog_ret)
{
  GLuint prog;
  GLint success;
  // FIXME: make this dynamic
  char compile_log[512];

  // Link program
  prog = glCreateProgram();
  glAttachShader(prog, vsh);
  glAttachShader(prog, fsh);
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
    return 1;
  }

  // We don't need these anymore
  glDeleteShader(vsh);
  glDeleteShader(fsh);
  printf("[%s] Program link succeeded\n",
    __func__);
  *prog_ret = prog;
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

  // Compile shaders
  shader_compile("vertex.vert", &vertexShader, GL_VERTEX_SHADER);
  shader_compile("fragment.frag", &fragmentShader, GL_FRAGMENT_SHADER);

  // Link shader program
  shader_link(vertexShader, fragmentShader, &shaderProgram);

  float vertices[] = {
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    0.0f,  1.0f, 0.0f
  };  

  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*) 0);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);

  glUseProgram(shaderProgram);

  glutDisplayFunc(draw_callback);
  glutIdleFunc(glutPostRedisplay);
  glutMainLoop();

  return 0;
}
