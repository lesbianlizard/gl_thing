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

static GLint win = 0;
volatile int colormode = 0;

unsigned int
  VAO,
  VBO,
  vertexShader,
  fragmentShader,
  shaderProgram;
float color_anim = 1;
float color_step = 0.01;
float color_dir = 1;

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

int mmap_file_cstr(char *name, int *fd_ret, char **cstr_ret)
{
  int fd = open(name, O_RDONLY);

  if (fd > 0)
  {
    int len = lseek(fd, 0, SEEK_END);
    *cstr_ret = mmap(0, len, PROT_READ, MAP_PRIVATE, fd, 0);
    *fd_ret = fd;
    return 0;
  }
  else
  {
    fprintf(stderr, _("[%1$s] error %2$i opening file '%3$s': %4$s\n"),
      __func__,
      errno,
      name,
      strerror(errno));
    return -1;
  }
}

int main(int argc, char **argv)
{
  int
    vsh_file_fd,
    fsh_file_fd,
    success;
  char
    *vsh_file,
    *fsh_file,
    compile_log[512];

  // Try to memory map shaders into c strings
  if (mmap_file_cstr("vertex.vert", &vsh_file_fd, &vsh_file) < 0)
  {
    return 1;
  }

  if (mmap_file_cstr("fragment.frag", &fsh_file_fd, &fsh_file) < 0)
  {
    return 1;
  }

  // FIXME: properly init window size with vars and stuff
  //glutInitWindowSize(800, 600);
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  win = glutCreateWindow("sort of a glxgears clone");
  //glViewport(0, 0, 800, 600);
  
  glewInit();

  printf("GL_VERSION  : %s\n", glGetString(GL_VERSION) );
  printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER) );

  
  // Compile vertex shader
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vsh_file, NULL);
  glCompileShader(vertexShader);

  // Check if vertex shader is ok
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(vertexShader, 512, NULL, compile_log);
    printf("[%s] vertex shader compile failed with status %i, log:\n%s",
      __func__,
      success,
      compile_log);
    return 1;
  }


  // Compile fragment shader
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fsh_file, NULL);
  glCompileShader(fragmentShader);

  // Check if fragment shader is ok
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(fragmentShader, 512, NULL, compile_log);
    printf("[%s] fragment shader compile failed with status %i, log:\n%s",
      __func__,
      success,
      compile_log);
    return 1;
  }

  close(vsh_file_fd);
  close(fsh_file_fd);


  // Link shader program
  shaderProgram = glCreateProgram();
  glAttachShader(shaderProgram, vertexShader);
  glAttachShader(shaderProgram, fragmentShader);
  glLinkProgram(shaderProgram);
  // We don't need these anymore
  glDeleteShader(vertexShader);
  glDeleteShader(fragmentShader);

  // Check if program link is ok 
  glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
  if (!success)
  {
  glGetProgramInfoLog(shaderProgram, 512, NULL, compile_log);
    printf("[%s] program link failed with status %i, log:\n%s",
      __func__,
      success,
      compile_log);
    return 1;
  }

  float vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f,  0.5f, 0.0f
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
