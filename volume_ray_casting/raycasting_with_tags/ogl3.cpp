/*


  Simple Demo for GLSL

  www.lighthouse3d.com

*/
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GL/glut.h>

#include <iostream>
#include <vector>
#include <limits>
using namespace std;

/// NVIDIA OpenGL SDK
#include <nvGlutManipulators.h>
#include <nvGlutWidgets.h>

#include "../my_raycasting/textfile.h"
#include "../my_raycasting/filename_utility.h"

/**
/* filename can be set in command arguments
/* in Visual Studio, it is in the project's Properties->Debugging->Command Arguments
*/
char volume_filename[MAX_STR_SIZE] = "data\\nucleon.dat";

/// call finailize() to free the memory before exit
void ** data_ptr = NULL;
GLenum gl_type;
int sizes[3];
int color_omponent_number;

/// for UI widgets
bool ui_on = true;
#define MAX_KEYS 256
#define WINDOW_SIZE 800
bool gKeys[MAX_KEYS];

/// diameter of the model
float diameter;
// center of the model
nv::vec3f center(0.0f, 0.0f, 0.0f);

/// UI widgets and trackball manipulator
/// model controller
nv::GlutExamine manipulator;
/// ui context
nv::GlutUIContext ui;

GLuint v,f,f2,p;
float lpos[4] = {1,0.5,1,0};

void changeSize(int w, int h)
{

    // Prevent a divide by zero, when window is too short
    // (you cant make a window of zero width).
    if(h == 0)
        h = 1;

    float ratio = 1.0* w / h;

    // Reset the coordinate system before modifying
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // Set the correct perspective.
    gluPerspective(45,ratio,1,1000);
    glMatrixMode(GL_MODELVIEW);


}
float a = 0;

void renderScene(void)
{

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();
    gluLookAt(0.0,0.0,5.0,
              0.0,0.0,-1.0,
              0.0f,1.0f,0.0f);

    glLightfv(GL_LIGHT0, GL_POSITION, lpos);
    glRotatef(a,0,1,1);
    glutSolidTeapot(1);
    a+=0.1;

    glutSwapBuffers();
}

void processNormalKeys(unsigned char key, int x, int y)
{

    if (key == 27)
        exit(0);
}

#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(char *file, int line)
{
    //
    // Returns 1 if an OpenGL error occurred, 0 otherwise.
    //
    GLenum glErr;
    int    retCode = 0;

    glErr = glGetError();
    while (glErr != GL_NO_ERROR)
    {
        printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
        retCode = 1;
        glErr = glGetError();
    }
    return retCode;
}


void printShaderInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}

void printProgramInfoLog(GLuint obj)
{
    int infologLength = 0;
    int charsWritten  = 0;
    char *infoLog;

    glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 0)
    {
        infoLog = (char *)malloc(infologLength);
        glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
        printf("%s\n",infoLog);
        free(infoLog);
    }
}



void setShaders()
{

    char *vs = NULL,*fs = NULL,*fs2 = NULL;

    v = glCreateShader(GL_VERTEX_SHADER);
    f = glCreateShader(GL_FRAGMENT_SHADER);
    f2 = glCreateShader(GL_FRAGMENT_SHADER);

    vs = file_utility::textFileRead("minimal.vert");
    fs = file_utility::textFileRead("minimal.frag");

    const char * vv = vs;
    const char * ff = fs;

    glShaderSource(v, 1, &vv,NULL);
    glShaderSource(f, 1, &ff,NULL);

    free(vs);
    free(fs);

    glCompileShader(v);
    glCompileShader(f);

    printShaderInfoLog(v);
    printShaderInfoLog(f);
    printShaderInfoLog(f2);

    p = glCreateProgram();
    glAttachShader(p,v);
    glAttachShader(p,f);

    glLinkProgram(p);
    printProgramInfoLog(p);

    glUseProgram(p);

}




int main(int argc, char **argv)
{
    filename_utility::print_about(argc, argv);

    // read filename from arguments if available
    if (argc > 1)
    {
        strcpy(volume_filename, argv[1]);
    }
    else
    {
        // read volume data filename from command line
        cout<<"Input data file: (for example, data\\nucleon.dat)"<<endl;
        cin>>volume_filename;
    }

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    //glutInitWindowPosition(100,100);
    glutInitWindowSize(WINDOW_SIZE,WINDOW_SIZE);

    char str[MAX_STR_SIZE];
    sprintf(str, "GPU raycasting - %s", volume_filename);
    glutCreateWindow(str);

    glutDisplayFunc(renderScene);
    glutIdleFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutKeyboardFunc(processNormalKeys);

    glEnable(GL_DEPTH_TEST);
    glClearColor(1.0,1.0,1.0,1.0);
    glEnable(GL_CULL_FACE);

    glewInit();
    if (glewIsSupported("GL_VERSION_2_0"))
        printf("Ready for OpenGL 2.0\n");
    else
    {
        printf("OpenGL 2.0 not supported\n");
        exit(1);
    }

    setShaders();

    glutMainLoop();

    return 0;
}

