
// --------------------------------------------------------------------------
// GPU raycasting tutorial
// Made by Peter Trier jan 2007
//
// This file contains all the elements nessesary to implement a simple 
// GPU volume raycaster.
// Notice this implementation requires a shader model 3.0 gfxcard
// --------------------------------------------------------------------------

#include <windows.h>
#include <GL/glew.h>
//#include <Cg/cg.h>
//#include <Cg/cgGL.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
#include <ctime>
#include <cassert>
#include <string>
#include "Vector3.h"
//////////////////////////////////////////////////////////////////////////
#include <memory>
#include "textfile.h"
#include "reader.h"
//////////////////////////////////////////////////////////////////////////

#define MAX_KEYS 256
#define WINDOW_SIZE 800
#define VOLUME_TEX_SIZE 128

using namespace std;

// Globals ------------------------------------------------------------------

bool gKeys[MAX_KEYS];
//bool toggle_visuals = true;
//////////////////////////////////////////////////////////////////////////
short toggle_visuals = 0;
//////////////////////////////////////////////////////////////////////////
//CGcontext context; 
//CGprofile vertexProfile, fragmentProfile; 
//CGparameter param1,param2;
GLuint renderbuffer; 
GLuint framebuffer; 
//CGprogram vertex_main,fragment_main; // the raycasting shader programs
GLuint volume_texture; // the volume texture
GLuint backface_buffer; // the FBO buffers
GLuint final_image;
float stepsize = 1.0/50.0;
//////////////////////////////////////////////////////////////////////////
GLuint frontface_buffer; // the FBO buffers
GLuint v,f,p;//,f2 // the OpenGL shaders
GLuint loc_stepsize;
GLuint volume_texture2; // the volume texture from files
//////////////////////////////////////////////////////////////////////////

/// Implementation ----------------------------------------

//////////////////////////////////////////////////////////////////////////
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

// Sets a uniform texture parameter
void setTextureUniform(GLuint program, const char* name, int number, GLenum target, GLuint texture) 
{
	GLuint location = glGetUniformLocation(program, name);
	glUniform1i(location, number);
	glActiveTexture(GL_TEXTURE0 + number);
	glBindTexture(target, texture);
}

void setShaders() {

	char *vs = NULL,*fs = NULL;// ,*fs2 = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	//f2 = glCreateShader(GL_FRAGMENT_SHADER);


	vs = textFileRead("simple_vertex.vert");
	//fs = textFileRead("simple_fragment.frag");
	fs = textFileRead("my_raycasting.frag");

	const char * vv = vs;
	const char * ff = fs;

	glShaderSource(v, 1, &vv,NULL);
	glShaderSource(f, 1, &ff,NULL);

	free(vs);free(fs);

	glCompileShader(v);
	glCompileShader(f);

	printShaderInfoLog(v);
	printShaderInfoLog(f);
	//printShaderInfoLog(f2);

	p = glCreateProgram();
	glAttachShader(p,v);
	glAttachShader(p,f);

	//Initial program setup.
	glLinkProgram(p); //Initial link

	glUseProgram(p);
	loc_stepsize = glGetUniformLocation(p, "stepsize");

	// set textures
	setTextureUniform(p, "front", 1, GL_TEXTURE_2D, frontface_buffer);
	setTextureUniform(p, "back", 2, GL_TEXTURE_2D, backface_buffer);
	setTextureUniform(p, "volume", 3, GL_TEXTURE_3D, volume_texture);
	setTextureUniform(p, "volume2", 4, GL_TEXTURE_3D, volume_texture2);

	// restore active texture unit to GL_TEXTURE0
	glActiveTexture(GL_TEXTURE0);
	// disable the shader program
	glUseProgram(0);
}

//////////////////////////////////////////////////////////////////////////

//void cgErrorCallback()
//{
//	CGerror lastError = cgGetError(); 
//	if(lastError)
//	{
//		cout << cgGetErrorString(lastError) << endl;
//		if(context != NULL)
//			cout << cgGetLastListing(context) << endl;
//		exit(0);
//	}
//}

//// Sets a uniform texture parameter
//void set_tex_param(char* par, GLuint tex,const CGprogram &program,CGparameter param) 
//{
//	param = cgGetNamedParameter(program, par); 
//	cgGLSetTextureParameter(param, tex); 
//	cgGLEnableTextureParameter(param);
//}

//// load_vertex_program: loading a vertex program
//void load_vertex_program(CGprogram &v_program,char *shader_path, char *program_name)
//{
//	assert(cgIsContext(context));
//	v_program = cgCreateProgramFromFile(context, CG_SOURCE,shader_path,
//		vertexProfile,program_name, NULL);	
//	if (!cgIsProgramCompiled(v_program))
//		cgCompileProgram(v_program);
//
//	cgGLEnableProfile(vertexProfile);
//	cgGLLoadProgram(v_program);
//	cgGLDisableProfile(vertexProfile);
//}

//// load_fragment_program: loading a fragment program
//void load_fragment_program(CGprogram &f_program,char *shader_path, char *program_name)
//{
//	assert(cgIsContext(context));
//	f_program = cgCreateProgramFromFile(context, CG_SOURCE, shader_path,
//		fragmentProfile,program_name, NULL);	
//	if (!cgIsProgramCompiled(f_program))
//		cgCompileProgram(f_program);
//
//	cgGLEnableProfile(fragmentProfile);
//	cgGLLoadProgram(f_program);
//	cgGLDisableProfile(fragmentProfile);
//}

void enable_renderbuffers()
{
	glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, framebuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer);
}

void disable_renderbuffers()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

//////////////////////////////////////////////////////////////////////////
int face_index = 0;
//////////////////////////////////////////////////////////////////////////
void vertex(float x, float y, float z)
{
	//////////////////////////////////////////////////////////////////////////
	float s, t;
	switch(face_index)
	{
	case 1:
		s = x;
		t = z;
		break;
	case 2:
		s = x;
		t = y;
		break;
	default:
		s = y;
		t = z;
	}
	glMultiTexCoord2f(GL_TEXTURE0, s, t);
	//////////////////////////////////////////////////////////////////////////
	glColor3f(x,y,z);
	glMultiTexCoord3f(GL_TEXTURE1, x, y, z);
	glVertex3f(x,y,z);
}
// this method is used to draw the front and backside of the volume
void drawQuads(float x, float y, float z)
{

	glBegin(GL_QUADS);
	/* Back side */
	//////////////////////////////////////////////////////////////////////////
	face_index = 2;
	//////////////////////////////////////////////////////////////////////////
	glNormal3f(0.0, 0.0, -1.0);
	vertex(0.0, 0.0, 0.0);
	vertex(0.0, y, 0.0);
	vertex(x, y, 0.0);
	vertex(x, 0.0, 0.0);

	/* Front side */
	//////////////////////////////////////////////////////////////////////////
	face_index = 2;
	//////////////////////////////////////////////////////////////////////////
	glNormal3f(0.0, 0.0, 1.0);
	vertex(0.0, 0.0, z);
	vertex(x, 0.0, z);
	vertex(x, y, z);
	vertex(0.0, y, z);

	/* Top side */
	//////////////////////////////////////////////////////////////////////////
	face_index = 1;
	//////////////////////////////////////////////////////////////////////////
	glNormal3f(0.0, 1.0, 0.0);
	vertex(0.0, y, 0.0);
	vertex(0.0, y, z);
	vertex(x, y, z);
	vertex(x, y, 0.0);

	/* Bottom side */
	//////////////////////////////////////////////////////////////////////////
	face_index = 1;
	//////////////////////////////////////////////////////////////////////////
	glNormal3f(0.0, -1.0, 0.0);
	vertex(0.0, 0.0, 0.0);
	vertex(x, 0.0, 0.0);
	vertex(x, 0.0, z);
	vertex(0.0, 0.0, z);

	/* Left side */
	//////////////////////////////////////////////////////////////////////////
	face_index = 0;
	//////////////////////////////////////////////////////////////////////////
	glNormal3f(-1.0, 0.0, 0.0);
	vertex(0.0, 0.0, 0.0);
	vertex(0.0, 0.0, z);
	vertex(0.0, y, z);
	vertex(0.0, y, 0.0);

	/* Right side */
	//////////////////////////////////////////////////////////////////////////
	face_index = 0;
	//////////////////////////////////////////////////////////////////////////
	glNormal3f(1.0, 0.0, 0.0);
	vertex(x, 0.0, 0.0);
	vertex(x, y, 0.0);
	vertex(x, y, z);
	vertex(x, 0.0, z);
	glEnd();

}

// create a test volume texture, here you could load your own volume
void create_volumetexture()
{
	int size = VOLUME_TEX_SIZE*VOLUME_TEX_SIZE*VOLUME_TEX_SIZE* 4;
	GLubyte *data = new GLubyte[size];

	for(int x = 0; x < VOLUME_TEX_SIZE; x++)
	{for(int y = 0; y < VOLUME_TEX_SIZE; y++)
	{for(int z = 0; z < VOLUME_TEX_SIZE; z++)
	{
		data[(x*4)   + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = z%250;
		data[(x*4)+1 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = y%250;
		data[(x*4)+2 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
		data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 230;

		Vector3 p =	Vector3(x,y,z)- Vector3(VOLUME_TEX_SIZE-20,VOLUME_TEX_SIZE-30,VOLUME_TEX_SIZE-30);
		bool test = (p.length() < 42);
		if(test)
			data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 0;

		p =	Vector3(x,y,z)- Vector3(VOLUME_TEX_SIZE/2,VOLUME_TEX_SIZE/2,VOLUME_TEX_SIZE/2);
		test = (p.length() < 24);
		if(test)
			data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 0;


		if(x > 20 && x < 40 && y > 0 && y < VOLUME_TEX_SIZE && z > 10 &&  z < 50)
		{

			data[(x*4)   + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 100;
			data[(x*4)+1 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
			data[(x*4)+2 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = y%100;
			data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
		}

		if(x > 50 && x < 70 && y > 0 && y < VOLUME_TEX_SIZE && z > 10 &&  z < 50)
		{

			data[(x*4)   + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
			data[(x*4)+1 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
			data[(x*4)+2 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = y%100;
			data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
		}

		if(x > 80 && x < 100 && y > 0 && y < VOLUME_TEX_SIZE && z > 10 &&  z < 50)
		{

			data[(x*4)   + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
			data[(x*4)+1 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 70;
			data[(x*4)+2 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = y%100;
			data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
		}

		p =	Vector3(x,y,z)- Vector3(24,24,24);
		test = (p.length() < 40);
		if(test)
			data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 0;
	}}}

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glGenTextures(1, &volume_texture);
	glBindTexture(GL_TEXTURE_3D, volume_texture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0,GL_RGBA, VOLUME_TEX_SIZE, VOLUME_TEX_SIZE,VOLUME_TEX_SIZE,0, GL_RGBA, GL_UNSIGNED_BYTE,data);

	delete []data;
	cout << "volume texture created" << endl;
}

// ok let's start things up 

void readVolumeFile(char* filename) 
{
	int sizes[3];
	float dists[3];
	std::tr1::shared_ptr<void *> data_ptr(new void *);
	DataType type;
	int numComponents;

	readData(filename, sizes, dists, data_ptr.get(), &type, &numComponents);

	GLenum glType;
	switch (type)
	{
	case DATRAW_UCHAR:
		glType = GL_UNSIGNED_BYTE;
		break;
	case DATRAW_USHORT:
		glType = GL_UNSIGNED_SHORT;
		break;
	default:
		char s[300] = "Unsupported data type in ";
		strcat(s, filename);
		throw std::exception(s);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glGenTextures(1, &volume_texture2);
	glBindTexture(GL_TEXTURE_3D, volume_texture2);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, numComponents, sizes[0], sizes[1], sizes[2], 0, GL_LUMINANCE, glType, *data_ptr.get());

	free(*data_ptr.get());
	cout << "volume texture created from " << filename << endl;
}

void init()
{
	cout << "glew init " << endl;
	GLenum err = glewInit();

	// initialize all the OpenGL extensions
	glewGetExtension("glMultiTexCoord2fvARB");  
	if(glewGetExtension("GL_EXT_framebuffer_object") )cout << "GL_EXT_framebuffer_object support " << endl;
	if(glewGetExtension("GL_EXT_renderbuffer_object"))cout << "GL_EXT_renderbuffer_object support " << endl;
	if(glewGetExtension("GL_ARB_vertex_buffer_object")) cout << "GL_ARB_vertex_buffer_object support" << endl;
	if(GL_ARB_multitexture)cout << "GL_ARB_multitexture support " << endl;

	if (glewGetExtension("GL_ARB_fragment_shader")      != GL_TRUE ||
		glewGetExtension("GL_ARB_vertex_shader")        != GL_TRUE ||
		glewGetExtension("GL_ARB_shader_objects")       != GL_TRUE ||
		glewGetExtension("GL_ARB_shading_language_100") != GL_TRUE)
	{
		cout << "Driver does not support OpenGL Shading Language" << endl;
		exit(1);
	}

	glEnable(GL_CULL_FACE);
	glClearColor(0.0, 0.0, 0.0, 0);
	create_volumetexture();

	//// CG init
	//cgSetErrorCallback(cgErrorCallback);
	//context = cgCreateContext();
	//if (cgGLIsProfileSupported(CG_PROFILE_VP40))
	//{
	//	vertexProfile = CG_PROFILE_VP40;
	//	cout << "CG_PROFILE_VP40 supported." << endl; 
	//}
	//else 
	//{
	//	if (cgGLIsProfileSupported(CG_PROFILE_ARBVP1))
	//		vertexProfile = CG_PROFILE_ARBVP1;
	//	else
	//	{
	//		cout << "Neither arbvp1 or vp40 vertex profiles supported on this system." << endl;
	//		exit(1);
	//	}
	//}

	//if (cgGLIsProfileSupported(CG_PROFILE_FP40))
	//{
	//	fragmentProfile = CG_PROFILE_FP40;
	//	cout << "CG_PROFILE_FP40 supported." << endl; 
	//}
	//else 
	//{
	//	if (cgGLIsProfileSupported(CG_PROFILE_ARBFP1))
	//		fragmentProfile = CG_PROFILE_ARBFP1;
	//	else
	//	{
	//		cout << "Neither arbfp1 or fp40 fragment profiles supported on this system." << endl;
	//		exit(1);
	//	}
	//}

	//// load the vertex and fragment raycasting programs
	//load_vertex_program(vertex_main,"raycasting_shader.cg","vertex_main");
	//cgErrorCallback();
	//load_fragment_program(fragment_main,"raycasting_shader.cg","fragment_main");
	//cgErrorCallback();

	// Create the to FBO's one for the backside of the volumecube and one for the finalimage rendering
	glGenFramebuffersEXT(1, &framebuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,framebuffer);

	//////////////////////////////////////////////////////////////////////////
	glGenTextures(1, &frontface_buffer);
	glBindTexture(GL_TEXTURE_2D, frontface_buffer);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA16F_ARB, WINDOW_SIZE, WINDOW_SIZE, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, frontface_buffer, 0);
	//////////////////////////////////////////////////////////////////////////

	glGenTextures(1, &backface_buffer);
	glBindTexture(GL_TEXTURE_2D, backface_buffer);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA16F_ARB, WINDOW_SIZE, WINDOW_SIZE, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, backface_buffer, 0);

	glGenTextures(1, &final_image);
	glBindTexture(GL_TEXTURE_2D, final_image);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA16F_ARB, WINDOW_SIZE, WINDOW_SIZE, 0, GL_RGBA, GL_FLOAT, NULL);

	glGenRenderbuffersEXT(1, &renderbuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, WINDOW_SIZE, WINDOW_SIZE);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, renderbuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	//////////////////////////////////////////////////////////////////////////
	// read volume data file
	readVolumeFile("data\\head256.dat");
	// init shaders
	setShaders();
	//////////////////////////////////////////////////////////////////////////
}


// for contiunes keypresses
void ProcessKeys()
{
	// Process keys
	for (int i = 0; i < 256; i++)
	{
		if (!gKeys[i])  { continue; }
		switch (i)
		{
		case ' ':
			break;
		case 'w':
			stepsize += 1.0/2048.0;
			if(stepsize > 0.25) stepsize = 0.25;
			break;
		case 'e':
			stepsize -= 1.0/2048.0;
			if(stepsize <= 1.0/200.0) stepsize = 1.0/200.0;
			break;
		}
	}

}

void key(unsigned char k, int x, int y)
{
	gKeys[k] = true;
}

void KeyboardUpCallback(unsigned char key, int x, int y)
{
	gKeys[key] = false;

	switch (key)
	{
	case 27 :
		{
			exit(0); break; 
		}
	case ' ':
		//toggle_visuals = !toggle_visuals;
		//////////////////////////////////////////////////////////////////////////
		toggle_visuals = (toggle_visuals + 1) % 3;
		//////////////////////////////////////////////////////////////////////////
		break;
	}
}

// glut idle function
void idle_func()
{
	ProcessKeys();
	glutPostRedisplay();
}

void reshape_ortho(int w, int h)
{
	if (h == 0) h = 1;
	glViewport(0, 0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);
	glMatrixMode(GL_MODELVIEW);
}


void resize(int w, int h)
{
	if (h == 0) h = 1;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 0.01, 400.0);
	glMatrixMode(GL_MODELVIEW);
}

void draw_fullscreen_quad()
{
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_QUADS);

	glTexCoord2f(0,0); 
	glVertex2f(0,0);

	glTexCoord2f(1,0); 
	glVertex2f(1,0);

	glTexCoord2f(1, 1); 
	glVertex2f(1, 1);

	glTexCoord2f(0, 1); 
	glVertex2f(0, 1);

	glEnd();
	glEnable(GL_DEPTH_TEST);

}

// display the final image on the screen
void render_buffer_to_screen()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	//if(toggle_visuals)
	//	glBindTexture(GL_TEXTURE_2D,final_image);
	//else
	//	glBindTexture(GL_TEXTURE_2D,backface_buffer);
	//////////////////////////////////////////////////////////////////////////
	switch(toggle_visuals)
	{
	case 1:
		glBindTexture(GL_TEXTURE_2D,backface_buffer);
		break;
	case 2:
		glBindTexture(GL_TEXTURE_2D,frontface_buffer);
		break;
	default:
		glBindTexture(GL_TEXTURE_2D,final_image);
	}
	//////////////////////////////////////////////////////////////////////////
	reshape_ortho(WINDOW_SIZE,WINDOW_SIZE);
	draw_fullscreen_quad();
	glDisable(GL_TEXTURE_2D);
}

//////////////////////////////////////////////////////////////////////////
// render the frontface to the offscreen buffer backface_buffer
void render_frontface()
{
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, frontface_buffer, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	drawQuads(1.0,1.0, 1.0);
	glDisable(GL_CULL_FACE);
}
//////////////////////////////////////////////////////////////////////////

// render the backface to the offscreen buffer backface_buffer
void render_backface()
{
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, backface_buffer, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	drawQuads(1.0,1.0, 1.0);
	glDisable(GL_CULL_FACE);
}

void raycasting_pass()
{
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, final_image, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	//cgGLEnableProfile(vertexProfile);
	//cgGLEnableProfile(fragmentProfile);
	//cgGLBindProgram(vertex_main);
	//cgGLBindProgram(fragment_main);
	//cgGLSetParameter1f( cgGetNamedParameter( fragment_main, "stepsize") , stepsize);
	//set_tex_param("tex",backface_buffer,fragment_main,param1);
	//set_tex_param("volume_tex",volume_texture,fragment_main,param2);
	//////////////////////////////////////////////////////////////////////////
	glUseProgram(p);
	glUniform1f(loc_stepsize, stepsize);
	//////////////////////////////////////////////////////////////////////////
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	drawQuads(1.0,1.0, 1.0);
	glDisable(GL_CULL_FACE);
	//cgGLDisableProfile(vertexProfile);
	//cgGLDisableProfile(fragmentProfile);
	//////////////////////////////////////////////////////////////////////////
	glUseProgram(0);
	//////////////////////////////////////////////////////////////////////////
}

// This display function is called once pr frame 
void display()
{
	static float rotate = 0; 
	rotate += 0.25;

	resize(WINDOW_SIZE,WINDOW_SIZE);
	enable_renderbuffers();

	glLoadIdentity();
	glTranslatef(0,0,-2.25);
	glRotatef(rotate,0,1,1);
	glTranslatef(-0.5,-0.5,-0.5); // center the texturecube
	//////////////////////////////////////////////////////////////////////////
	render_frontface();
	//////////////////////////////////////////////////////////////////////////
	render_backface();
	raycasting_pass();
	disable_renderbuffers();
	render_buffer_to_screen();
	glutSwapBuffers();
}

int main(int argc, char* argv[])
{
	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("GPU raycasting tutorial");
	glutReshapeWindow(WINDOW_SIZE,WINDOW_SIZE);
	glutKeyboardFunc(key);
	glutKeyboardUpFunc(KeyboardUpCallback);

	glutDisplayFunc(display);
	glutIdleFunc(idle_func);
	glutReshapeFunc(resize);
	resize(WINDOW_SIZE,WINDOW_SIZE);
	init();
	glutMainLoop();
	return 0;
}
