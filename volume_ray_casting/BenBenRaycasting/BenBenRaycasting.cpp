/**	@file
*	GPU raycasting with transfer functions
*	
*	GPU raycasting tutorial
*	http://www.daimi.au.dk/~trier/?page_id=98
*/

// --------------------------------------------------------------------------
// GPU raycasting tutorial
// Made by Peter Trier jan 2007
//
// This file contains all the elements nessesary to implement a simple 
// GPU volume raycaster.
// Notice this implementation requires a shader model 3.0 gfxcard
// --------------------------------------------------------------------------

/**	@mainpage	GPU Volume Raycasting by Xiao Li (c) 2010-2011
*	GPU raycasting tutorial made by Peter Trier jan 2007
*	
*	This file contains all the elements nessesary to implement a simple 
*	GPU volume raycaster.
*	Notice this implementation requires a shader model 3.0 gfxcard.
*	
*	Adapted by Shengzhou Luo (ark) 2010-2011.
*	Transfer functions are wirtten by Xiao Li (Ben) 2010-2011.
*		
*	The program is implemented using OpenGL and GLSL (OpenGL Shading Language). Properties such as average, variation and local entropy of each voxel are pre-computed because they are constants during the rendering process and a higher frame rate could be reached.
*	The program is run on a personal computer (AMD Athlon 7750 Dual-Core Processor, 4G memory) equipped with NVIDIA GeForce GT 240 graphics card. Several common datasets that are publicly available on the The Volume Library is tested.
*	The original datasets in pvm format are converted into raw format with the pvm tools distributed with the V^3 (Versatile Volume Viewer) volume rendering package.
*
*	Last updated: 2011-8-5
*/

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
#include "color.h"
//#include "reader.h"
#include "volume.h"
#include "k_means.h"
// NVIDIA OpenGL SDK
#include <nvGlutManipulators.h>
#include <nvGlutWidgets.h>
#include <nvVector.h>

//////////////////////////////////////////////////////////////////////////
#include "../my_raycasting/textfile.h"
#include "../my_raycasting/VolumeReader.h"

#include "transfer_function.h"
//////////////////////////////////////////////////////////////////////////
/**	@brief MAX_KEYS == 256
*/
#define MAX_KEYS 256

/**	@brief WINDOW_SIZE == 700
*/
#define WINDOW_SIZE 700

/**	@brief VOLUME_TEX_SIZE == 128
*/
#define VOLUME_TEX_SIZE 128

using namespace std;

// Globals ------------------------------------------------------------------
bool gKeys[MAX_KEYS];
int width = WINDOW_SIZE, height = WINDOW_SIZE;
bool full_screen = false;
double a = 0.5, b = 0.6;
double epsilon = 10;
double max_distance;
float luminance = 10;
int low_hue = 0, high_hue =360;
//bool toggle_visuals = true;

//////////////////////////////////////////////////////////////////////////
short toggle_visuals = 0;
//////////////////////////////////////////////////////////////////////////

//CGcontext context; 
//CGprofile vertexProfile, fragmentProfile; 
//CGparameter param1,param2;
GLuint renderbuffer; 
GLuint framebuffer; 
GLuint loc_luminance;
//CGprogram vertex_main,fragment_main; // the raycasting shader programs
GLuint volume_texture; // the volume texture
GLuint transfer_texture; //transfer function texture
GLuint volume2;
GLuint backface_buffer; // the FBO buffers
GLuint final_image;
float stepsize = 1.0/1000.0;
//////////////////////////////////////////////////////////////////////////
GLuint frontface_buffer; // the FBO buffers
GLuint v,f,p; // the OpenGL shaders
GLuint loc_stepsize;

//////////////////////////////////////////////////////////////////////////
// added by ark @ 2010.10.15
volume_utility::VolumeReader volume;

//////////////////////////////////////////////////////////////////////////
/// for ui and user interaction
nv::GlutExamine manipulator;
nv::GlutUIContext ui;

//////////////////////////////////////////////////////////////////////////
// added by ark @ 2010.10.15
char volume_filename[MAX_STR_SIZE] = "data\\nucleon.dat";
//////////////////////////////////////////////////////////////////////////

/// record clusters
char * lable;

/// test if voxels comply to normal distribution
void NormalTest(void);

/**	@brief select user interested area using v, bounding_angle, r1 and r2
*/
void select_user_interested_area(Vector3 v, float bounding_angle, float r1, float r2);

/**	@brief updateButtonState
*/
inline void updateButtonState( const nv::ButtonState &bs, nv::GlutManipulator &manip, int button)
{
	int modMask = 0;

	if (bs.state & nv::ButtonFlags_Alt) modMask |= GLUT_ACTIVE_ALT;
	if (bs.state & nv::ButtonFlags_Shift) modMask |= GLUT_ACTIVE_SHIFT;
	if (bs.state & nv::ButtonFlags_Ctrl) modMask |= GLUT_ACTIVE_CTRL;
	if (bs.state & nv::ButtonFlags_End)
		manip.mouse( button, GLUT_UP, modMask, bs.cursor.x, height - bs.cursor.y);
	if (bs.state & nv::ButtonFlags_Begin)
		manip.mouse( button, GLUT_DOWN, modMask, bs.cursor.x, height - bs.cursor.y);
}

/**	@brief updateButtonState
*/
void doUI()
{
	nv::Rect null;

	ui.begin();

	//ui.beginGroup( nv::GroupFlags_GrowDownFromLeft);

	//ui.doLabel( null, "Cg Geometry Program");
	//if (ui.doComboBox( null, NPROGS, programs, &current_prog_idx)) {
	//	current_prog = &prog[current_prog_idx];
	//}
	//ui.doCheckButton( null, "Wireframe", &options[OPTION_DISPLAY_WIREFRAME]);
	//ui.doCheckButton( null, "Use Model", &options[OPTION_DRAW_MODEL]);

	//ui.doLabel( null, "Num Curve Segments");

	//ui.beginGroup( nv::GroupFlags_GrowLeftFromTop);
	//ui.doLabel( null, " ");
	//float fsegments = segments;
	//ui.doHorizontalSlider( null, 2.0f, 100.0f, &fsegments);
	//segments = fsegments;
	//ui.endGroup();

	//ui.endGroup();
	nv::Rect full_slider(-5,0,800,0);
	ui.doHorizontalSlider(full_slider, 0.00001, 100, &luminance);
	//	sprintf(str, "Luminance: %f", luminance);

	// Pass non-ui mouse events to the manipulator
	if (!ui.isOnFocus()) {
		const nv::ButtonState &lbState = ui.getMouseState( 0);
		const nv::ButtonState &mbState = ui.getMouseState( 1);
		const nv::ButtonState &rbState =  ui.getMouseState( 2);

		manipulator.motion( ui.getCursorX(), height - ui.getCursorY());

		updateButtonState( lbState, manipulator, GLUT_LEFT_BUTTON);
		updateButtonState( mbState, manipulator, GLUT_MIDDLE_BUTTON);
		updateButtonState( rbState, manipulator, GLUT_RIGHT_BUTTON);
	}

	ui.end();
}

///intesity max, gradient magnitude max, second derivative max, third derivative max
int data_max, grad_max, df2_max, df3_max;
color_opacity * tf = NULL;

//////////////////////////////////////////////////////////////////////////
/// Implementation ----------------------------------------
//////////////////////////////////////////////////////////////////////////

/// print shader information log to report any error that occurs  
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

/// print shader's information
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

/// Sets a uniform texture parameter
void setTextureUniform(GLuint program, const char* name, int number, GLenum target, GLuint texture) 
{
	GLuint location = glGetUniformLocation(program, name);
	glUniform1i(location, number);
	glActiveTexture(GL_TEXTURE0 + number);
	glBindTexture(target, texture);
}

/// initialize shaders
void setShaders() {

	char *vs = NULL,*fs = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	vs = file_reader::textFileRead("simple_vertex.vert.cc");
	fs = file_reader::textFileRead("BenBenRaycasting.frag.cc");

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
	glUniform1f(loc_luminance, luminance);
	loc_stepsize = glGetUniformLocation(p, "stepsize");

	// set textures
	setTextureUniform(p, "front", 1, GL_TEXTURE_2D, frontface_buffer);
	setTextureUniform(p, "back", 2, GL_TEXTURE_2D, backface_buffer);
	setTextureUniform(p, "volume_texture", 3, GL_TEXTURE_3D, volume_texture);
	setTextureUniform(p, "transfer_texture", 4, GL_TEXTURE_3D, transfer_texture);
	setTextureUniform(p, "volume2", 5, GL_TEXTURE_3D, volume2);
	loc_luminance = glGetUniformLocation(p, "luminance");
	// restore active texture unit to GL_TEXTURE0
	glActiveTexture(GL_TEXTURE0);
	// disable the shader program
	glUseProgram(0);
}

/// render images to buffers
void enable_renderbuffers()
{
	glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, framebuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer);
}

/// disable render buffers
void disable_renderbuffers()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

/// face index for setting a vertex
int face_index = 0;

/// draw a vertex
void vertex(float x, float y, float z)
{
	// set 2D texture coordinates for texture 0
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

	// set 3D texture coordinates for texture 1
	glColor3f(x,y,z);
	glMultiTexCoord3f(GL_TEXTURE1, x, y, z);
	glVertex3f(x,y,z);
}

/// this method is used to draw the front and backside of the volume
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

// ok let's start things up 

/*void readVolumeFile(char* filename) 
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
glGenTextures(1, &volume_texture);
glBindTexture(GL_TEXTURE_3D, volume_texture);
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
*/


///create volume texture
void create_volume_texture()
{
	GLenum glType;

	// test volume's data type and get glType
	if(strcmp(volume.getFormat(), "UCHAR") == 0) 
		glType = GL_UNSIGNED_BYTE;
	else if(strcmp(volume.getFormat(), "USHORT") ==0)
		glType = GL_UNSIGNED_SHORT;
	else
		cout<<"Invalid data type"<<endl;
	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glGenTextures(1, &volume_texture);
	glBindTexture(GL_TEXTURE_3D, volume_texture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, volume.getX(), volume.getY(), volume.getZ(), 0, GL_LUMINANCE, glType, volume.getDataAddr());
}

/// create a test volume texture, here you could load your own volume
void create_volumetexture()
{
	int size = VOLUME_TEX_SIZE*VOLUME_TEX_SIZE*VOLUME_TEX_SIZE* 4;
	GLubyte *data = new GLubyte[size];

	for(int x = 0; x < VOLUME_TEX_SIZE; x++)
	{
		for(int y = 0; y < VOLUME_TEX_SIZE; y++)
		{
			for(int z = 0; z < VOLUME_TEX_SIZE; z++)
			{
				data[(x*4)   + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = z%250;
				data[(x*4)+1 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = y%250;
				data[(x*4)+2 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 250;
				data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 230;

				nv::vec3f p =	nv::vec3f(x,y,z)- nv::vec3f(VOLUME_TEX_SIZE-20,VOLUME_TEX_SIZE-30,VOLUME_TEX_SIZE-30);
				bool test = (length(p) < 42);
				if(test)
					data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 0;

				p =	nv::vec3f(x,y,z)- nv::vec3f(VOLUME_TEX_SIZE/2,VOLUME_TEX_SIZE/2,VOLUME_TEX_SIZE/2);
				test = (length(p) < 24);
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

				p =	nv::vec3f(x,y,z)- nv::vec3f(24,24,24);
				test = (length(p) < 40);
				if(test)
					data[(x*4)+3 + (y * VOLUME_TEX_SIZE * 4) + (z * VOLUME_TEX_SIZE * VOLUME_TEX_SIZE * 4)] = 0;
			}
		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT,1);
	glGenTextures(1, &volume2);
	glBindTexture(GL_TEXTURE_3D, volume2);
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


///create tranfser function texture
void create_transferfunc()
{
	// set transfer function to be used
	setTransferfunc3(tf, volume);

	// bind transfer function texture 
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &transfer_texture);
	glBindTexture(GL_TEXTURE_3D, transfer_texture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, volume.getX(), volume.getY(), volume.getZ(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tf);

	// free the transfer function pointer after texture mapping
	free_transfer_function_pointer(tf);
}

/// do initilization work
void init()
{
	int x, y, z, index;
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
	glClearColor(0, 0, 0, 0);
	//		glClearColor(1, 1, 1, 1);
	glGenFramebuffersEXT(1, &framebuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,framebuffer);

	//////////////////////////////////////////////////////////////////////////
	// bind front face 
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
	// bind back face texture
	glGenTextures(1, &backface_buffer);
	glBindTexture(GL_TEXTURE_2D, backface_buffer);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA16F_ARB, WINDOW_SIZE, WINDOW_SIZE, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, backface_buffer, 0);

	// bind final image texture
	glGenTextures(1, &final_image);
	glBindTexture(GL_TEXTURE_2D, final_image);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA16F_ARB, WINDOW_SIZE, WINDOW_SIZE, 0, GL_RGBA, GL_FLOAT, NULL);

	//bind render buffer texture
	glGenRenderbuffersEXT(1, &renderbuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer);
	glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, WINDOW_SIZE, WINDOW_SIZE);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, renderbuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	volume.readVolFile(volume_filename);

	//do some calculations if necessary

	//volume.average_deviation();
	//volume.calLocalEntropy();
	//volume.calAverage();
	//volume.calVariation();
	//	Volume.calHistogram();
	//	Volume.calEp();
	//	Volume.NormalDistributionTest();
	//volume.calGrad();
	volume.calGrad();
	//	volume.calGrad_ex();
	volume.calDf2();
	//	volume.average_deviation();
	//	NormalTest();
	//	Volume.calLH();
	//	Volume.calDf3();
	//	Volume.Intensity_gradient_histogram();
	//	Volume.statistics();
	//	Volume.cluster();
	//	volume.getInfo();

	/*lable = (char *) malloc(sizeof(char) * Volume.getCount());
	if(lable == NULL)
	cout<<"Not enought space for lable"<<endl;
	memset(lable, '255', Volume.getCount());*/
	//	k_means(&Volume, lable);
	create_volume_texture();


	//	cout<<"local entropy max ="<<volume.getLocalEntropyMax()<<endl;

	create_transferfunc();

	//	create_volumetexture();
	// init shaders
	setShaders();
	//	//////////////////////////////////////////////////////////////////////////
}

/// for continues key presses
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

/// handle keys to be pressed
void key(unsigned char k, int x, int y)
{
	gKeys[k] = true;
	if(k == 'a')
	{
		a += 0.5;
		setTransferfunc(tf, volume);
		glutPostRedisplay();
	}	

	if(k == 's')
	{
		a -= 0.5;
		setTransferfunc(tf, volume);
		glutPostRedisplay();
	}
	if(k == 'd')
	{
		b += 0.5;
		setTransferfunc(tf, volume);
		glutPostRedisplay();
	}
	if(k == 'f')
	{
		b -= 0.5;
		setTransferfunc(tf, volume);
		glutPostRedisplay();
	}
	if(k == 'H')
	{
		high_hue+=5;
		setTransferfunc(tf, volume);
		glutPostRedisplay();
	}
	if(k == 'h')
	{
		high_hue -=5;
		setTransferfunc(tf, volume);
		glutPostRedisplay();
	}
	if(k == 'L')
	{
		low_hue +=5;
		setTransferfunc(tf, volume);
		glutPostRedisplay();
	}
	if(k == 'l')
	{
		low_hue -=5;
		setTransferfunc(tf, volume);
		glutPostRedisplay();
	}

}

///keyboard callback function
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

/// glut idle function
void idle_func()
{
	manipulator.idle();

	ProcessKeys();
	glutPostRedisplay();
}

/// set projection mode to Ortho 2D
void reshape_ortho(int w, int h)
{
	if (h == 0) h = 1;
	glViewport(0, 0,w,h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);
	glMatrixMode(GL_MODELVIEW);
}

/// resize the window
void resize(int w, int h)
{
	if (h == 0) h = 1;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 0.01, 400.0);
	glMatrixMode(GL_MODELVIEW);

	manipulator.reshape(w, h);
}

/// draw full screen quads
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

/// display the final image on the screen
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
// render the front face to the off screen buffer backface_buffer
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
/// render the back face to the off screen buffer backface_buffer
void render_backface()
{
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, backface_buffer, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	drawQuads(1.0,1.0, 1.0);
	glDisable(GL_CULL_FACE);
}

/// raycasting 
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
	drawQuads(1.0, 1.0, 1.0);
	glDisable(GL_CULL_FACE);
	//cgGLDisableProfile(vertexProfile);
	//cgGLDisableProfile(fragmentProfile);
	//////////////////////////////////////////////////////////////////////////
	glUseProgram(0);
	//////////////////////////////////////////////////////////////////////////
}

/// This display function is called once pr frame 
void display()
{
	static float rotate = 0; 
	rotate += 0.25;

	resize(WINDOW_SIZE,WINDOW_SIZE);
	enable_renderbuffers();

	glLoadIdentity();
	//glTranslatef(0,0,-2.25);
	//glRotatef(rotate,0,1,1);
	manipulator.applyTransform();
	glTranslatef(-0.5,-0.5,-0.5); // center the texturecube
	//////////////////////////////////////////////////////////////////////////
	render_frontface();
	//////////////////////////////////////////////////////////////////////////
	render_backface();
	raycasting_pass();
	disable_renderbuffers();
	render_buffer_to_screen();

	doUI();
	glutSwapBuffers();
}

/// handle mouse's movement
void mouse(int button, int state, int x, int y)
{
	ui.mouse( button, state, glutGetModifiers(), x, y);
}

/// handle mouse interaction
void motion(int x, int y)
{
	ui.mouseMotion( x, y);
}

/// handle special keys pressed
void special_keys(int key, int x, int y)
{
	switch(key)
	{
	case GLUT_KEY_F1:
		full_screen = !full_screen;
		if(full_screen)
		{
			width = glutGet(GLUT_WINDOW_WIDTH);
			height = glutGet(GLUT_WINDOW_HEIGHT);
			glutFullScreen();
		}
		else
			glutReshapeWindow(width, height);
		break;
	default:
		break;
	}
}

/// test if voxels comply to normal distribution
void NormalTest()
{
	int x, y, z, i, j, k;
	int index, count = 0;
	int value[27], temp;
	float a[13] = { 0.4366, 0.3018, 0.2522, 0.2152, 0.1848, 0.1584, 0.1346,
		0.1128, 0.0923, 0.0728, 0.0540, 0.0358, 0.0178
	};
	float average, d1, d2, w;
	for(x = 1; x <= volume.getX() - 1; ++x)
		for(y = 1;y <= volume.getY() - 1; ++y)
			for(z = 1; z  <= volume.getZ() - 1; ++z)
			{
				index = 1;
				for(i = x - 1; i <= x + 1; ++i)
					for(j = y - 1; j <= y +1; ++j)
						for(k = z - 1; k <= z + 1; ++k)
						{
							value[index] = volume.getData(i, j, k);
							index++;
						}
						for(i = 0; i < 27; ++i)
							for(j = i + 1;j < 27; ++j)
							{
								if(value[j] < value[i])
								{
									temp = value[j];
									value[j] = value[i];
									value[i] = temp;
								}
							}
							average = 0;
							for(i = 0; i < 27;++i)
							{
								average += float(value[i]);
								//		cout<<value[i]<<"\t";
							}

							average /= 27;
							d1 = d2 = 0;
							for(i = 0;i < 13; ++i)
								d1 += a[i] * double(value[27 - i] - value[i]);
							d1 = d1 * d1;
							for(i = 0;i < 27;++i)
								d2 += float(value[i] - average) * float(value[i] - average);
							if(d2 == 0)
								w = 2;
							else
								w = d1 / d2;
							//		cout<<"w = "<<w<<endl;
							if(w - 0.894 < 0)     
								// 0.923 0.935
								cout<<"Not comply to normal distribution"<<endl;
							else
							{
								count++;
								cout<<"Comply to normal distribution"<<endl<<endl<<endl;
							}

			}
			cout<<float(count) /  (float(volume.getX()) * float(volume.getY()) * float(volume.getZ()));
}

/// the program's entry function
int main(int argc, char* argv[])
{
	// print about information
	filename_utility::print_about(argc, argv);

	// get volume filename from arguments or console input
	filename_utility::get_filename(argc, argv, volume_filename);

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutInitWindowPosition(50, 50);
	//	glutInitWindowSize(700, 700);
	glutCreateWindow("BenBen Raycasting");

	glutReshapeWindow(WINDOW_SIZE,WINDOW_SIZE);

	manipulator.setDollyActivate( GLUT_LEFT_BUTTON, GLUT_ACTIVE_CTRL);
	manipulator.setPanActivate( GLUT_LEFT_BUTTON, GLUT_ACTIVE_SHIFT);
	manipulator.setDollyPosition( -2.5f  );

	///keyboard
	glutKeyboardFunc(key);
	glutKeyboardUpFunc(KeyboardUpCallback);

	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutIdleFunc(idle_func);
	glutPassiveMotionFunc(motion);
	glutReshapeFunc(resize);
	glutSpecialFunc(special_keys);
	resize(800,600);
	init();
	glutMainLoop();

	// added by ark @ 2011.04.26
	// free the transfer function pointer before exit
	free_transfer_function_pointer(tf);

	return 0;
}

/// select user's interested area, denoted by v, bounding_angle, r1 and r2
void select_user_interested_area(Vector3 v, float bounding_angle, float r1, float r2)
{
	int x, y, z, dim_x, dim_y, dim_z, index;
	float gx, gy, gz, g;
	float angle;
	float dot_product;

	angle = bounding_angle / 180.0 * pi;
	dim_x = volume.getX();
	dim_y = volume.getY();
	dim_z = volume.getZ();

	for(x = 0; x < dim_x; ++x)
		for(y = 0;y < dim_y; ++y)
			for(z = 0; z < dim_z; ++z)
			{
				index = volume.getIndex(x, y, z);		
				{
					if(x == 0)
						gx = float(volume.getData(x + 1, y, z) - volume.getData(x, y, z));
					else if(x == dim_x - 1)
						gx = float(volume.getData(x, y, z) - volume.getData(x - 1, y, z));
					else
						gx = float(volume.getData(x + 1, y, z)) - float(volume.getData(x - 1, y, z));

					if(y == 0)
						gy = float(volume.getData(x , y + 1, z) - volume.getData(x, y, z));
					else if(y == dim_y - 1)
						gy = float(volume.getData(x, y, z) - volume.getData(x, y - 1, z));
					else
						gy = float(volume.getData(x , y + 1, z)) - float(volume.getData(x , y - 1, z));

					if(z == 0)
						gz = float(volume.getData(x, y, z + 1) - volume.getData(x, y, z));
					else if(z == dim_z - 1)
						gz =float(volume.getData(x, y, z) - volume.getData(x, y, z - 1));
					else
						gz = float(volume.getData(x , y, z + 1)) - float(volume.getData(x , y, z - 1));

					g = sqrt(gx * gx + gy * gy + gz * gz);
					gx /= g;
					gy /= g;
					gz /= g;

					dot_product = v.x() * gx + v.y() * gy + v.z() * gz;

					if((dot_product <= cos(angle)) && (g >= r1) && (g <= r2))
					{
						;
						//		tf[index].a =255;
					}
					else
					{
						tf[index].a = 0;
						//		tf[index].r = tf[index].g = tf[index].b = 1;
					}
				}
			}
}
