//#include <GL/glew.h>
//#include <GLFW/glfw3.h>
#include <GL/gl.h>
#include <GL/freeglut.h>

#include <stdio.h>

static GLint win = 0;


//void framebuffer_size_callback(GLFWwindow* window, int width, int height)
//{
//  glViewport(0, 0, width, height);
//}  

static void draw_callback(void)
{
  glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
  //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glClear(GL_COLOR_BUFFER_BIT);

  glutSwapBuffers();
}

int main(int argc, char** argv)
{
//  glfwInit();
//  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
//  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
//  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);


  //GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
  glutInitWindowSize(800, 600);

  //glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);  
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
  win = glutCreateWindow("sort of a glxgears clone");
  


//  if (window == NULL)
//  {
//    printf("Failed to create GLFW window\n");
//    glfwTerminate();
//    return -1;
//  }

  //glfwMakeContextCurrent(window);

  printf("GL_VERSION  : %s\n", glGetString(GL_VERSION) );
  printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER) );

  glViewport(0, 0, 800, 600);

  
  float vertices[] = {
    -0.5f, -0.5f, 0.0f,
    0.5f, -0.5f, 0.0f,
    0.0f,  0.5f, 0.0f
  };  


//  while(!glfwWindowShouldClose(window))
//  {

    glutDisplayFunc(draw_callback);
    glutMainLoop();

//    glfwSwapBuffers(window);
//    glfwPollEvents();    
//  }


//  glfwTerminate();
  return 0;
}
