#include <GL/gl.h>
#include <GL/freeglut.h>
#include <stdio.h>

static GLint win = 0;
volatile int colormode = 0;

static void draw_callback(void)
{
  printf("[%s] starting, colormode = %i\n", __func__, colormode);
  glClear(GL_COLOR_BUFFER_BIT);

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
    
  glutSwapBuffers();
}

int main(int argc, char** argv)
{
  glutInitWindowSize(800, 600);
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  win = glutCreateWindow("sort of a glxgears clone");
  glViewport(0, 0, 800, 600);

  printf("GL_VERSION  : %s\n", glGetString(GL_VERSION) );
  printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER) );

  float vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f,  0.5f, 0.0f
  };  

  glutDisplayFunc(draw_callback);
  glutIdleFunc(glutPostRedisplay);
  glutMainLoop();
  return 0;
}
