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

static void draw_callback(void)
{
  //printf("[%s] starting, colormode = %i\n", __func__, colormode);

  if (colormode == 0)
  {
    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    colormode = 1;
  }
  else
  {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    colormode = 0;
  }
    
  glClear(GL_COLOR_BUFFER_BIT);
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
  int vsh_file_fd, fsh_file_fd;
  char *vsh_file, *fsh_file;

  if (mmap_file_cstr("vertex.vert", &vsh_file_fd, &vsh_file) < 0)
  {
    return 1;
  }

  if (mmap_file_cstr("fragment.frag", &fsh_file_fd, &fsh_file) < 0)
  {
    return 1;
  }

  glutInitWindowSize(800, 600);
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  win = glutCreateWindow("sort of a glxgears clone");
  glViewport(0, 0, 800, 600);
  
  glewInit();

  printf("GL_VERSION  : %s\n", glGetString(GL_VERSION) );
  printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER) );

  float vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f,  0.5f, 0.0f
  };  

  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  
  unsigned int vertexShader;
  vertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertexShader, 1, &vsh_file, NULL);
  glCompileShader(vertexShader);

  int vsh_compile_success;
  char vsh_compile_log[512];
  glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &vsh_compile_success);
  glGetShaderInfoLog(vertexShader, 512, NULL, vsh_compile_log);
  printf("[%s] vertex shader compile status %i, log:\n%s", __func__, vsh_compile_success, vsh_compile_log);


  unsigned int fragmentShader;
  fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragmentShader, 1, &fsh_file, NULL);
  glCompileShader(fragmentShader);

  int fsh_compile_success;
  char fsh_compile_log[512];
  glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fsh_compile_success);
  glGetShaderInfoLog(fragmentShader, 512, NULL, fsh_compile_log);
  printf("[%s] fragment shader compile status %i, log:\n%s", __func__, fsh_compile_success, fsh_compile_log);

  close(vsh_file_fd);
  close(fsh_file_fd);


  glutDisplayFunc(draw_callback);
  glutIdleFunc(glutPostRedisplay);
  glutMainLoop();
  return 0;
}
