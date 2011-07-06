/**	@file
*	GPU raycasting with segmentation tags
*/

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

#include "../BenBenRaycasting/transfer_function.h"
#include "../my_raycasting/VolumeReader.h"
#include "../my_raycasting/textfile.h"
#include "../my_raycasting/filename_utility.h"
#include "../my_raycasting/volume_utility.h"
#include "reader_tag.h"
#include "tag.h"

/**
/* filename can be set in command arguments
/* in Visual Studio, it is in the project's Properties->Debugging->Command Arguments
*/
char volume_filename[MAX_STR_SIZE] = "data\\nucleon.dat";

/// call finailize() to free the memory before exit
void ** data_ptr = NULL;
GLenum gl_type;
int sizes[3];
int color_component_number;

GLuint loc_sizes;

/// buffers, textures
GLuint renderbuffer; 
GLuint framebuffer; 

/// the volume texture
GLuint volume_texture;

/// the FBO buffers
GLuint backface_buffer;

/// the FBO buffers
GLuint frontface_buffer;

/// buffer for the 2D transfer function
GLuint transfer_function_2D_buffer;

/// buffer for the histogram
GLuint histogram_buffer;

/// buffer for the gradient histogram
GLuint histogram_gradient_buffer;
GLuint final_image;

/// the volume texture from files
GLuint volume_texture_from_file;
GLuint transfer_texture;
GLuint gradient_texture;

// texture for segmentation tags
GLuint tag_texture;

/// for shaders
const float STEPSIZE_MAX = 1.0/4.0;
const float STEPSIZE_MIN = 1e-4;
const float STEPSIZE_INC = STEPSIZE_MIN;
float stepsize = 1.0/100.0;

/// GLSL shaders
GLuint v,f,p;

GLuint loc_stepsize;
GLuint loc_volume_texture_from_file;
GLuint loc_transfer_texture;
GLuint loc_gradient_texture;
GLuint loc_tag_texture;
const float LUMINANCE_MAX = 200;
const float LUMINANCE_MIN = 1;
const float LUMINANCE_INC = 1;
float luminance = 1;
GLuint loc_luminance;

/// for UI widgets
bool ui_on = true;
#define MAX_KEYS 256
#define WINDOW_SIZE 800
#define VOLUME_TEX_SIZE 128
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

/// for button widgets
bool button_auto_rotate = false;
bool button_lock_viewpoint = false;
bool button_generate_Ben_transfer_function = false;

/// for output image
enum RenderOption
{
	RENDER_FINAL_IMAGE,
	RENDER_BACK_FACE,
	RENDER_FRONT_FACE,
	RENDER_TRANSFER_FUNCTION_2D,
	RENDER_HISTOGRAM,
	RENDER_HISTOGRAM_GRADIENT,
	RENDER_COUNT
};
int render_option = RENDER_FINAL_IMAGE;

/// for transfer function
enum TransferFunctionOption
{
	TRANSFER_FUNCTION_NONE,
	TRANSFER_FUNCTION_2D,
	TRANSFER_FUNCTION_BEN,
	TRANSFER_FUNCTION_TAG,
	TRANSFER_FUNCTION_SOBEL_3D,
	TRANSFER_FUNCTION_SOBEL_3D_SCALAR,
	TRANSFER_FUNCTION_COUNT
};
int transfer_function_option = TRANSFER_FUNCTION_NONE;
GLuint loc_transfer_function_option;

// for lighting
float fSpecularPower = 25;
float fvLightPosition[3] = {-100, 100, 100};
float fvEyePosition[3] = {0, 0, 100};
float fvAmbient[4] = {0.368627, 0.368421, 0.368421, 1.0};
float fvSpecular[4] = {0.886275, 0.885003, 0.885003, 1.0};
float fvDiffuse[4] = {0.490196, 0.488722, 0.488722, 1.0};
GLuint loc_fSpecularPower;
GLuint loc_fvLightPosition;
GLuint loc_fvEyePosition;
GLuint loc_fvAmbient;
GLuint loc_fvSpecular;
GLuint loc_fvDiffuse;

/************************************************************************/
/*	for ui widgets                                                      */
/************************************************************************/

/// GLUT callback function, for special keys
void special(int c, int x, int y) {
	ui.keyboard(c, x, y);
}

/// update button states for ui widgets
inline void updateButtonState(const nv::ButtonState &bs, nv::GlutManipulator &manip, int button) {
	int modMask = 0;

	if (bs.state & nv::ButtonFlags_Alt) modMask |= GLUT_ACTIVE_ALT;
	if (bs.state & nv::ButtonFlags_Shift) modMask |= GLUT_ACTIVE_SHIFT;
	if (bs.state & nv::ButtonFlags_Ctrl) modMask |= GLUT_ACTIVE_CTRL;

	if (bs.state & nv::ButtonFlags_End)
		manip.mouse(button, GLUT_UP, modMask, bs.cursor.x, WINDOW_SIZE - bs.cursor.y);
	if (bs.state & nv::ButtonFlags_Begin)
		manip.mouse(button, GLUT_DOWN, modMask, bs.cursor.x, WINDOW_SIZE - bs.cursor.y);
}

/// draw ui widgets
void doUI()
{
	nv::Rect none;
	const char *render_str[RENDER_COUNT] = {"Final image", "Back faces", "Front faces", "2D transfer function", "Histogram", "Gradient"};
	const char *transfer_function_str[TRANSFER_FUNCTION_COUNT] = {"No transfer function", "2D", "Ben", "Tags", "Sobel 3D", "Sobel 3D scalar"};

	glDisable(GL_CULL_FACE);

	ui.begin();

	if (ui_on)
	{
		ui.beginGroup();

		ui.beginGroup(nv::GroupFlags_GrowRightFromBottom|nv::GroupFlags_LayoutNoMargin);
		//ui.doCheckButton(none, "Test cube", &button_show_generated_cube);
		ui.doCheckButton(none, "Rotate", &button_auto_rotate);
		ui.doCheckButton(none, "View lock", &button_lock_viewpoint);
		//ui.doCheckButton(none, "Alpha blend", &button_show_alpha_blending);
		//ui.doButton(none, "Generate histogram", &button_generate_histogram);
		//ui.doButton(none, "Cluster", &button_cluster);
		ui.doButton(none, "Ben TF", &button_generate_Ben_transfer_function);
		//ui.doButton(none, "Fusion TF", &button_generate_fusion_transfer_function);
		//ui.doButton(none, "Do all", &button_all);
		//ui.doButton(none, "Load label", &button_load_importance_label);
		ui.endGroup();

		ui.doComboBox(none, RENDER_COUNT, render_str, &render_option);
		//ui.doComboBox(none, PEELING_COUNT, peeling_str, &peeling_option);
		ui.doComboBox(none, TRANSFER_FUNCTION_COUNT, transfer_function_str, &transfer_function_option);

		//ui.doLineEdit(none, text, MAX_STR_SIZE, &chars_returned);

		ui.endGroup();

		ui.beginGroup(nv::GroupFlags_GrowDownFromRight);
		char str[MAX_STR_SIZE];
		sprintf(str, "Step size: %f", stepsize);
		ui.doLabel(none, str);

		nv::Rect rect_slider(0,0,600,0);
		ui.doHorizontalSlider(rect_slider, STEPSIZE_MIN, STEPSIZE_MAX, &stepsize);

		//// show peeling widgets
		//switch(peeling_option)
		//{
		//case PEELING_OPACITY_IMPORTANCE:
		//case PEELING_OPACITY:
		//	// if(accumulated>high && sampled<low)
		//	sprintf(str, "peeling condition: accumulated>high && sampled<low    threshold low: %f threshold high: %f", threshold_low, threshold_high);
		//	ui.doLabel(none, str);
		//	ui.doHorizontalSlider(rect_slider, OPACITY_THRESHOLD_MIN, OPACITY_THRESHOLD_MAX, &threshold_low);
		//	ui.doHorizontalSlider(rect_slider, OPACITY_THRESHOLD_MIN, OPACITY_THRESHOLD_MAX, &threshold_high);
		//	break;
		//case PEELING_FEATURE:
		//	sprintf(str, "slope threshold: %f", slope_threshold);
		//	ui.doLabel(none, str);
		//	ui.doHorizontalSlider(rect_slider, SLOPE_THRESHOLD_MIN, SLOPE_THRESHOLD_MAX, &slope_threshold);
		//	break;
		//case PEELING_GRADIENT_IMPORTANCE:
		//case PEELING_GRADIENT:
		//	// if(accumulated>high && sampled<low)
		//	sprintf(str, "peeling condition: accumulated>high && sampled<low    threshold low: %f threshold high: %f", threshold_low, threshold_high);
		//	ui.doLabel(none, str);
		//	ui.doHorizontalSlider(rect_slider, GRADIENT_THRESHOLD_MIN, GRADIENT_THRESHOLD_MAX, &threshold_low);
		//	ui.doHorizontalSlider(rect_slider, GRADIENT_THRESHOLD_MIN, GRADIENT_THRESHOLD_MAX, &threshold_high);
		//	break;
		//}
		//switch(peeling_option)
		//{
		//case PEELING_OPACITY:
		//case PEELING_FEATURE:
		//case PEELING_BACK:
		//case PEELING_FRONT:
		//case PEELING_GRADIENT:
		//case PEELING_OPACITY_IMPORTANCE:
		//case PEELING_GRADIENT_IMPORTANCE:
		//	sprintf(str, "peeling_layer: %f", peeling_layer);
		//	ui.doLabel(none, str);
		//	ui.doHorizontalSlider(rect_slider, LAYER_MIN, LAYER_MAX, &peeling_layer);
		//	break;
		//}

		//// show transfer function widgets
		//if (transfer_function_option == TRANSFER_FUNCTION_K_MEANS || transfer_function_option == TRANSFER_FUNCTION_K_MEANS_EQUALIZED)
		//{
		//	sprintf(str, "k-means k=%f", cluster_quantity);
		//	ui.doLabel(none, str);
		//	ui.doHorizontalSlider(rect_slider, 1, 16, &cluster_quantity);
		//}else
		//{
		//	if (button_show_alpha_blending && (transfer_function_option == TRANSFER_FUNCTION_SOBEL || transfer_function_option == TRANSFER_FUNCTION_SOBEL_3D))
		//	{
		//		sprintf(str, "opacity=mix(gradient,scalar,alpha)    alpha=%f", alpha_opacity);
		//		ui.doLabel(none, str);
		//		ui.doHorizontalSlider(rect_slider, 0, 1, &alpha_opacity);
		//	}else
		//	{
		//		if (transfer_function_option == TRANSFER_FUNCTION_FUSION)
		//		{
		//			sprintf(str, "fusion factor = %f", fusion_factor);
		//			ui.doLabel(none, str);
		//			ui.doHorizontalSlider(rect_slider, 0, 1, &fusion_factor);
		//		}
		//	}
		//}

		ui.endGroup();

		ui.beginGroup(nv::GroupFlags_GrowUpFromLeft);
		nv::Rect full_slider(-5,0,800,0);
		ui.doHorizontalSlider(full_slider, LUMINANCE_MIN, LUMINANCE_MAX, &luminance);
		sprintf(str, "Luminance: %f", luminance);
		ui.doLabel(none, str);
		//ui.doHorizontalSlider(full_slider, CLIP_MIN, CLIP_MAX, &clip);
		//sprintf(str, "Clip: %f", clip);
		//ui.doLabel(none, str);
		//if (render_option == RENDER_HISTOGRAM)
		//{
		//	ui.doHorizontalSlider(full_slider, 0.00001, 1.0, &picked);
		//	sprintf(str, "Histogram value picked: %f", picked);
		//	ui.doLabel(none, str);
		//}
		ui.endGroup();
	}

	// Pass non-ui mouse events to the manipulator
	if (!ui.isOnFocus()) {
		const nv::ButtonState &lbState = ui.getMouseState(0);
		const nv::ButtonState &mbState = ui.getMouseState(1);
		const nv::ButtonState &rbState =  ui.getMouseState(2);

		if (!button_lock_viewpoint)
		{
			manipulator.motion(ui.getCursorX(), WINDOW_SIZE - ui.getCursorY());
		}

		updateButtonState(lbState, manipulator, GLUT_LEFT_BUTTON);
		updateButtonState(mbState, manipulator, GLUT_MIDDLE_BUTTON);
		updateButtonState(rbState, manipulator, GLUT_RIGHT_BUTTON);
	}

	ui.end();
}

/// GLUT callback function, trigger when mouse buttons are pressed
void mouse(int button, int state, int x, int y) {
	ui.mouse(button, state, glutGetModifiers(), x, y);
}

/// GLUT callback function, trigger when mouse move and buttons are pressed
void motion(int x, int y) {
	ui.mouseMotion(x, y);
}

/// GLUT callback function, trigger when mouse moves
void passiveMotion(int x, int y) {
	ui.mouseMotion(x, y);
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

/// Sets a uniform texture parameter
GLuint add_texture_uniform(GLuint program, const char* name, int number, GLenum target, GLuint texture) 
{
	GLuint location = glGetUniformLocation(program, name);
	glUniform1i(location, number);
	glActiveTexture(GL_TEXTURE0 + number);
	glBindTexture(target, texture);

	// restore active texture unit to GL_TEXTURE0
	glActiveTexture(GL_TEXTURE0);
	return location;
}

/// set texture uniform
void set_texture_uniform(GLuint location, GLuint program, const char* name, int number, GLenum target, GLuint texture) 
{
	glUniform1i(location, number);
	glActiveTexture(GL_TEXTURE0 + number);
	glBindTexture(target, texture);

	// restore active texture unit to GL_TEXTURE0
	glActiveTexture(GL_TEXTURE0);
}

/// load shaders from files and set shaders
void setShaders()
{
	char *vs = NULL,*fs = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	vs = file_utility::textFileRead("simple_vertex.vert.cc");
	fs = file_utility::textFileRead("raycasting_with_tags.frag.cc");

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

	p = glCreateProgram();
	glAttachShader(p,v);
	glAttachShader(p,f);

	// Initial program setup.
	glLinkProgram(p); // Initial link
	printProgramInfoLog(p);

	// use the shader program
	glUseProgram(p);
	loc_stepsize = glGetUniformLocation(p, "stepsize");
	//loc_threshold_high = glGetUniformLocation(p, "threshold_high");
	//loc_threshold_low = glGetUniformLocation(p, "threshold_low");
	//loc_peeling_option = glGetUniformLocation(p, "peeling_option");
	loc_transfer_function_option = glGetUniformLocation(p, "transfer_function_option");
	loc_luminance = glGetUniformLocation(p, "luminance");
	loc_sizes = glGetUniformLocation(p, "sizes");
	//loc_clip = glGetUniformLocation(p, "clip");
	//loc_slope_threshold = glGetUniformLocation(p, "slope_threshold");
	//loc_cluster_interval = glGetUniformLocation(p, "cluster_interval");
	//loc_peeling_layer = glGetUniformLocation(p, "peeling_layer");
	//loc_scalar_min_normalized = glGetUniformLocation(p, "scalar_min_normalized");
	//loc_scalar_max_normalized = glGetUniformLocation(p, "scalar_max_normalized");
	//loc_alpha_opacity = glGetUniformLocation(p, "alpha_opacity");
	//loc_fusion_factor = glGetUniformLocation(p, "fusion_factor");

	// for lighting
	loc_fSpecularPower = glGetUniformLocation(p, "fSpecularPower");
	loc_fvLightPosition = glGetUniformLocation(p, "fvLightPosition");
	loc_fvEyePosition = glGetUniformLocation(p, "fvEyePosition");
	loc_fvAmbient = glGetUniformLocation(p, "fvAmbient");
	loc_fvSpecular = glGetUniformLocation(p, "fvSpecular");
	loc_fvDiffuse = glGetUniformLocation(p, "fvDiffuse");

	// set textures
	add_texture_uniform(p, "front", 1, GL_TEXTURE_2D, frontface_buffer);
	add_texture_uniform(p, "back", 2, GL_TEXTURE_2D, backface_buffer);
	loc_volume_texture_from_file = add_texture_uniform(p, "volume_texture", 3, GL_TEXTURE_3D, volume_texture_from_file);
	add_texture_uniform(p, "transfer_function_2D", 4, GL_TEXTURE_2D, transfer_function_2D_buffer);
	loc_transfer_texture = add_texture_uniform(p, "transfer_texture", 5, GL_TEXTURE_3D, transfer_texture);
	//loc_cluster_texture =  add_texture_uniform(p, "cluster_texture", 6, GL_TEXTURE_3D, cluster_texture);
	//loc_importance_texture =  add_texture_uniform(p, "importance_texture", 7, GL_TEXTURE_3D, importance_texture);
	//loc_transfer_texture2 = add_texture_uniform(p, "transfer_texture2", 8, GL_TEXTURE_3D, transfer_texture2);
	loc_tag_texture =  add_texture_uniform(p, "tag_texture", 6, GL_TEXTURE_3D, tag_texture);
	loc_gradient_texture =  add_texture_uniform(p, "gradient_texture", 7, GL_TEXTURE_3D, gradient_texture);

	// disable the shader program
	glUseProgram(0);
}

/// render images to buffers
void enable_renderbuffers()
{
	glBindFramebufferEXT (GL_FRAMEBUFFER_EXT, framebuffer);
	glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, renderbuffer);
}

/// disable rendering to buffers
void disable_renderbuffers()
{
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

/************************************************************************/
/// face index for setting a vertex
int face_index = 0;

/// draw a vertex
void vertex(float x, float y, float z)
{
	/************************************************************************/
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
	/************************************************************************/
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
	face_index = 2; // for setting 2D texture coordinates

	glNormal3f(0.0, 0.0, -1.0);
	vertex(0.0, 0.0, 0.0);
	vertex(0.0, y, 0.0);
	vertex(x, y, 0.0);
	vertex(x, 0.0, 0.0);

	/* Front side */
	face_index = 2; // for setting 2D texture coordinates

	glNormal3f(0.0, 0.0, 1.0);
	vertex(0.0, 0.0, z);
	vertex(x, 0.0, z);
	vertex(x, y, z);
	vertex(0.0, y, z);

	/* Top side */
	face_index = 1; // for setting 2D texture coordinates

	glNormal3f(0.0, 1.0, 0.0);
	vertex(0.0, y, 0.0);
	vertex(0.0, y, z);
	vertex(x, y, z);
	vertex(x, y, 0.0);

	/* Bottom side */
	face_index = 1; // for setting 2D texture coordinates

	glNormal3f(0.0, -1.0, 0.0);
	vertex(0.0, 0.0, 0.0);
	vertex(x, 0.0, 0.0);
	vertex(x, 0.0, z);
	vertex(0.0, 0.0, z);

	/* Left side */
	face_index = 0; // for setting 2D texture coordinates

	glNormal3f(-1.0, 0.0, 0.0);
	vertex(0.0, 0.0, 0.0);
	vertex(0.0, 0.0, z);
	vertex(0.0, y, z);
	vertex(0.0, y, 0.0);

	/* Right side */
	face_index = 0; // for setting 2D texture coordinates

	glNormal3f(1.0, 0.0, 0.0);
	vertex(x, 0.0, 0.0);
	vertex(x, y, 0.0);
	vertex(x, y, z);
	vertex(x, 0.0, z);
	glEnd();
}

/// increase a parameter
inline float increase(const float value, const float inc, const float max)
{
	float temp = value + inc;
	return temp>max ? max : temp;
}

/// decrease a parameter
inline float decrease(const float value, const float dec, const float min)
{
	float temp = value - dec;
	return temp<min ? min : temp;
}

/// for continue keypresses
void key_hold()
{
	// Process keys
	for (int i = 0; i < 256; i++)
	{
		if (!gKeys[i])  { continue; }
		switch (i)
		{
			//case ' ':
			//	break;
		case 'w':
			stepsize = increase(stepsize, STEPSIZE_INC, STEPSIZE_MAX);
			break;
		case 's':
			stepsize = decrease(stepsize, STEPSIZE_INC, STEPSIZE_MIN);
			break;
		case 'd':
			luminance = increase(luminance, LUMINANCE_INC, LUMINANCE_MAX);
			break;
		case 'a':
			luminance = decrease(luminance, LUMINANCE_INC, LUMINANCE_MIN);
			break;
		}
	}
}

/// GLUT callback function, trigger when keys are pressed
void key_press(unsigned char k, int x, int y)
{
	gKeys[k] = true;
}

/// GLUT callback function, trigger when keys are released
void key_release(unsigned char key, int x, int y)
{
	gKeys[key] = false;
	switch (key)
	{
	case 27 :
		// escape to exit
		exit(0);
		break;
	case 'r':
		button_auto_rotate = !button_auto_rotate;
		break;
	case 'v':
		button_lock_viewpoint = !button_lock_viewpoint;
		break;
	case 'i':
		// image to render
		if (glutGetModifiers() == GLUT_ACTIVE_ALT)
		{
			render_option = (render_option - 1 + RENDER_COUNT) % RENDER_COUNT;
		}else
		{
			render_option = (render_option + 1) % RENDER_COUNT;
		}
		break;
	case 'u':
		// turn UI on/off
		ui_on = !ui_on;
		break;
	case 't':
		// transfer function
		if (glutGetModifiers() == GLUT_ACTIVE_ALT)
		{
			transfer_function_option = (transfer_function_option - 1 + TRANSFER_FUNCTION_COUNT) % TRANSFER_FUNCTION_COUNT;
		}else
		{
			transfer_function_option = (transfer_function_option + 1) % TRANSFER_FUNCTION_COUNT;
		}
		break;
	}
}

/// GLUT callback function, trigger when the window is idle
void idle_func()
{
	if(button_auto_rotate)
	{
		// increment the rotation
		manipulator.idle();
	}

	key_hold();
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


/// GLUT callback function, trigger when the window's size change
void resize(int w, int h)
{
	if (h == 0) h = 1;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)w/(GLfloat)h, 0.01, 400.0);
	glMatrixMode(GL_MODELVIEW);

	ui.reshape(w, h);

	// pass the change along to the controller
	manipulator.reshape(w, h);
}

/// draw a fullscreen quadrangle, for the rendering result to display in
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

	// choose the buffer to render 
	switch(render_option)
	{
	case RENDER_FINAL_IMAGE:
		glBindTexture(GL_TEXTURE_2D, final_image);
		break;
	case RENDER_BACK_FACE:
		glBindTexture(GL_TEXTURE_2D, backface_buffer);
		break;
	case RENDER_FRONT_FACE:
		glBindTexture(GL_TEXTURE_2D, frontface_buffer);
		break;
	case RENDER_TRANSFER_FUNCTION_2D:
		glBindTexture(GL_TEXTURE_2D, transfer_function_2D_buffer);
		break;
	case RENDER_HISTOGRAM:
		glBindTexture(GL_TEXTURE_2D, histogram_buffer);
		break;
	case RENDER_HISTOGRAM_GRADIENT:
		glBindTexture(GL_TEXTURE_2D, histogram_gradient_buffer);
		break;
	default:
		std::cerr<<"Unknown Render Option!"<<endl;
	}

	reshape_ortho(WINDOW_SIZE,WINDOW_SIZE); // set projection mode to Ortho 2D
	draw_fullscreen_quad(); // draw a full screen quadrangle to show the content in the texture buffer
	glDisable(GL_TEXTURE_2D);
}

/// render the 2D transfer function
void render_transfer_function_2D()
{
	// how many colors
	const int N = 7;
	// the positions to draw quadrangle strips
	const float p[7] = {0, 1/6., 2/6., 3/6., 4/6., 5/6., 1};
	// the colors of the transfer function
	const float colors[7][3] = {
		1, 0, 0,
		1, 1, 0,
		0, 1, 0,
		0, 1, 1,
		0, 0, 1,
		1, 0, 1,
		1, 0, 0
	};

	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, transfer_function_2D_buffer, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();
	reshape_ortho(WINDOW_SIZE,WINDOW_SIZE); // set projection mode to Ortho 2D

	// draw a full screen quadrangle to show the content in the texture buffer
	glDisable(GL_DEPTH_TEST);
	glBegin(GL_QUAD_STRIP);
	for (unsigned int i = 0; i<N; i++)
	{
		glColor3fv(colors[i]);
		glVertex2f(p[i], 0);
		glColor3fv(colors[i]);
		glVertex2f(p[i], 1);
	}
	glEnd();
	glEnable(GL_DEPTH_TEST);

	resize(WINDOW_SIZE,WINDOW_SIZE); // set projection mode back to 3D
}

/// render the frontface to the offscreen buffer backface_buffer
void render_frontface()
{
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, frontface_buffer, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	drawQuads(1.0,1.0, 1.0);
	glDisable(GL_CULL_FACE);
}

/// render the backface to the offscreen buffer backface_buffer
void render_backface()
{
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, backface_buffer, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	drawQuads(1.0,1.0, 1.0);
	glDisable(GL_CULL_FACE);
}

/// load a transfer function by Ben
void create_transferfunc_Ben()
{
	volume_utility::VolumeReader volume;
	volume.readVolFile(volume_filename);
	volume.calHistogram();
	volume.calGrad_ex();
	volume.calDf2();
	volume.calDf3();
	volume.statistics();

	color_opacity * tf = NULL;
	setTransferfunc3(tf, volume);

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

/// a raycasting pass
void raycasting_pass()
{
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, final_image, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	//int peeling_layer_int = static_cast<int>(peeling_layer);
	//peeling_layer = peeling_layer_int;

	//int k = static_cast<int>(cluster_quantity);
	//cluster_quantity = k;
	//if (cluster_quantity_int_old != k)
	//{
	//	cluster_quantity_int_old = k;
	//	cluster_interval = volume_utility::get_cluster_interval(cluster_quantity_int_old);
	//}

	// Use shaders
	glUseProgram(p);
	glUniform1f(loc_stepsize, stepsize);
	//glUniform1f(loc_threshold_high, threshold_high);
	//glUniform1f(loc_threshold_low, threshold_low);
	glUniform1f(loc_luminance, luminance);
	//glUniform1f(loc_clip, clip);
	glUniform3f(loc_sizes, sizes[0], sizes[1], sizes[2]);
	//glUniform1f(loc_cluster_interval, cluster_interval);
	//glUniform1f(loc_scalar_min_normalized, scalar_min_normalized);
	//glUniform1f(loc_scalar_max_normalized, scalar_max_normalized);
	//glUniform1f(loc_slope_threshold, slope_threshold);
	//glUniform1f(loc_alpha_opacity, alpha_opacity);
	//glUniform1f(loc_fusion_factor, fusion_factor);

	//glUniform1i(loc_peeling_option, peeling_option);
	glUniform1i(loc_transfer_function_option, transfer_function_option);
	//glUniform1i(loc_peeling_layer, peeling_layer_int);

	// for lighting
	glUniform1f(loc_fSpecularPower, fSpecularPower);
	glUniform3fv(loc_fvLightPosition, 1, fvLightPosition);
	glUniform3fv(loc_fvEyePosition, 1, fvEyePosition);
	glUniform4fv(loc_fvAmbient, 1, fvAmbient);
	glUniform4fv(loc_fvSpecular, 1, fvSpecular);
	glUniform4fv(loc_fvDiffuse, 1, fvDiffuse);

	if(button_generate_Ben_transfer_function)
	{
		button_generate_Ben_transfer_function = false;
		create_transferfunc_Ben();
		set_texture_uniform(loc_transfer_texture, p, "transfer_texture", 5, GL_TEXTURE_3D, transfer_texture);
	}

	//if(button_generate_fusion_transfer_function)
	//{
	//	button_generate_fusion_transfer_function = false;
	//	create_transferfunc_fusion();
	//	set_texture_uniform(loc_transfer_texture, p, "transfer_texture", 5, GL_TEXTURE_3D, transfer_texture);
	//	set_texture_uniform(loc_transfer_texture2, p, "transfer_texture2", 8, GL_TEXTURE_3D, transfer_texture2);
	//}

	//if(button_show_generated_cube != button_show_generated_cube_backup)
	//{
	//	button_show_generated_cube_backup = button_show_generated_cube;
	//	if(button_show_generated_cube)
	//		set_texture_uniform(loc_volume, p, "volume", 3, GL_TEXTURE_3D, volume_texture);
	//	else
	//		set_texture_uniform(loc_volume, p, "volume", 3, GL_TEXTURE_3D, volume_texture_from_file);
	//}

	// draw front faces
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	drawQuads(1.0, 1.0, 1.0);
	glDisable(GL_CULL_FACE);

	// Disable shaders
	glUseProgram(0);
}

/// GLUT callback function. This display function is called once per frame
void display()
{
	static float rotate = 0; 
	rotate += 0.25;

	resize(WINDOW_SIZE,WINDOW_SIZE);
	enable_renderbuffers();
	render_transfer_function_2D();

	//// do it all
	//if(data_ptr && button_all)
	//{
	//	button_all = false;
	//	button_generate_histogram = true;
	//	button_cluster = true;
	//	//button_generate_Ben_transfer_function = true;
	//	button_generate_fusion_transfer_function = true;
	//}

	//// render the histogram
	//if(data_ptr && button_generate_histogram)
	//{
	//	button_generate_histogram = false;
	//	unsigned int count = sizes[0]*sizes[1]*sizes[2];
	//	if (gl_type == GL_UNSIGNED_SHORT)
	//		render_histograms<unsigned short, 65536>((unsigned short*)*data_ptr, count, color_omponent_number);
	//	else
	//		render_histograms<unsigned char, 256>((unsigned char*)*data_ptr, count, color_omponent_number);
	//}

	//// cluster
	//if (data_ptr && button_cluster)
	//{
	//	button_cluster = false;
	//	unsigned int count = sizes[0]*sizes[1]*sizes[2];
	//	if (gl_type == GL_UNSIGNED_SHORT)
	//		cluster<unsigned short, 65536>((unsigned short*)*data_ptr, count);
	//	else
	//		cluster<unsigned char, 256>((unsigned char*)*data_ptr, count);
	//}

	//// load importance label
	//if(data_ptr && button_load_importance_label)
	//{
	//	button_load_importance_label = false;
	//	load_importance_label(sizes[0]*sizes[1]*sizes[2]);
	//}

	glLoadIdentity();
	//glTranslatef(0,0,-2.25); // move the content backward, in order to show it
	//glRotatef(rotate,0,1,1);
	//glTranslatef(-0.5,-0.5,-0.5); // center the texturecube

	manipulator.applyTransform();
	glTranslatef(-center[0], -center[1], -center[2]); // center the texturecube

	render_frontface();
	render_backface();
	raycasting_pass();
	disable_renderbuffers();
	render_buffer_to_screen();
	doUI();
	glutSwapBuffers();
}

/// read volume data from file
void read_volume_file(char* filename)
{
	float dists[3];
	file_utility::DataType type;

	if (!data_ptr)
	{
		data_ptr = new void *;
	}
	file_utility::readData(filename, sizes, dists, data_ptr, &type, &color_component_number);

	switch (type)
	{
	case file_utility::DATRAW_UCHAR:
		gl_type = GL_UNSIGNED_BYTE;
		break;
	case file_utility::DATRAW_USHORT:
		gl_type = GL_UNSIGNED_SHORT;
		break;
	default:
		std::cerr<<"Unsupported data type in "<<filename<<endl;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &volume_texture_from_file);
	glBindTexture(GL_TEXTURE_3D, volume_texture_from_file);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, color_component_number, sizes[0], sizes[1], sizes[2], 0, GL_LUMINANCE, gl_type, *data_ptr);

	cout << "volume texture created from " << filename << endl;
}

/// read volume data from file with tags
void read_volume_file_with_tag(char* filename)
{
	float dists[3];
	file_utility::DataType type;

	if (!data_ptr)
	{
		data_ptr = new void *;
	}
	char tag_filename[MAX_STR_SIZE] = "";
	file_utility::readData_with_tag(filename, sizes, dists, data_ptr, &type, &color_component_number, tag_filename);

	switch (type)
	{
	case file_utility::DATRAW_UCHAR:
		gl_type = GL_UNSIGNED_BYTE;
		break;
	case file_utility::DATRAW_USHORT:
		gl_type = GL_UNSIGNED_SHORT;
		break;
	default:
		std::cerr<<"Unsupported data type in "<<filename<<endl;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &volume_texture_from_file);
	glBindTexture(GL_TEXTURE_3D, volume_texture_from_file);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, color_component_number, sizes[0], sizes[1], sizes[2], 0, GL_LUMINANCE, gl_type, *data_ptr);

	cout << "volume texture created from " << filename << endl;

	// load tags volume as texture
	if (strlen(tag_filename) > 0)
	{
		char tag_filename_with_path[MAX_STR_SIZE];
		filename_utility::get_path_from_other_filename(filename, tag_filename, tag_filename_with_path);
		std::cout<<"TaggedFileName: "<<tag_filename_with_path<<endl;
		int tag_sizes[3];
		float tag_dists[3];
		void ** tag_data_ptr = new void *;
		file_utility::DataType tag_type;
		int tag_color_component_number;
		file_utility::readData(tag_filename_with_path, tag_sizes, tag_dists, tag_data_ptr, &tag_type, &tag_color_component_number);

		unsigned int count = tag_sizes[0] * tag_sizes[1] * tag_sizes[2];
		GLenum tag_gl_type;
		switch (tag_type)
		{
		case file_utility::DATRAW_UCHAR:
			tag_gl_type = GL_UNSIGNED_BYTE;
			volume_utility::normalize_volume<unsigned char, 256>((unsigned char*)*tag_data_ptr, count, tag_color_component_number);
			break;
		case file_utility::DATRAW_USHORT:
			tag_gl_type = GL_UNSIGNED_SHORT;
			volume_utility::normalize_volume<unsigned short, 65536>((unsigned short*)*tag_data_ptr, count, tag_color_component_number);
			break;
		default:
			std::cerr<<"Unsupported data type in "<<filename<<endl;
		}

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glGenTextures(1, &tag_texture);
		glBindTexture(GL_TEXTURE_3D, tag_texture);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
		glTexImage3D(GL_TEXTURE_3D, 0, tag_color_component_number, tag_sizes[0], tag_sizes[1], tag_sizes[2], 0, GL_LUMINANCE, tag_gl_type, *tag_data_ptr);

		if (tag_data_ptr)
		{
			free(*tag_data_ptr);
			delete tag_data_ptr;
			tag_data_ptr = NULL;
		}
	}
}

/// estimate gradient texture
void estimate_gradient_texture()
{
	unsigned int count = sizes[0]*sizes[1]*sizes[2];
	unsigned short *gradient_data = new unsigned short[count * 3];

	vector<float> scalar_value(count); // the scalar data in const T *data
	vector<nv::vec3f> gradient(count);
	std::cout<<"Scalar histogram..."<<std::endl;

	if (gl_type == GL_UNSIGNED_SHORT)
	{
		unsigned int histogram[65536] = {0};
		volume_utility::generate_scalar_histogram<unsigned short, 65536>((unsigned short*)*data_ptr, count, color_omponent_number, histogram, scalar_value);
	}
	else
	{
		unsigned int histogram[256] = {0};
		volume_utility::generate_scalar_histogram<unsigned char, 256>((unsigned char*)*data_ptr, count, color_omponent_number, histogram, scalar_value);
	}

	volume_utility::estimate_gradient(gradient_data, sizes, count, scalar_value, gradient);

	delete [] gradient_data;
}

/// free the data pointer before exit
void finailize()
{
	if (data_ptr)
	{
		free(*data_ptr);
		delete data_ptr;
		data_ptr = NULL;
	}
}

/// ok let's start things up
void initialize()
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

	// black or white background
	glClearColor(0, 0, 0, 0);
	//glClearColor(1, 1, 1, 1);

	//create_volumetexture_a_cube();

	// Create the to FBO's one for the backside of the volumecube and one for the finalimage rendering
	glGenFramebuffersEXT(1, &framebuffer);
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT,framebuffer);

	glGenTextures(1, &histogram_gradient_buffer);
	glBindTexture(GL_TEXTURE_2D, histogram_gradient_buffer);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA16F_ARB, WINDOW_SIZE, WINDOW_SIZE, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, histogram_gradient_buffer, 0);

	glGenTextures(1, &histogram_buffer);
	glBindTexture(GL_TEXTURE_2D, histogram_buffer);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA16F_ARB, WINDOW_SIZE, WINDOW_SIZE, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, histogram_buffer, 0);

	glGenTextures(1, &transfer_function_2D_buffer);
	glBindTexture(GL_TEXTURE_2D, transfer_function_2D_buffer);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA16F_ARB, WINDOW_SIZE, WINDOW_SIZE, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, transfer_function_2D_buffer, 0);

	glGenTextures(1, &frontface_buffer);
	glBindTexture(GL_TEXTURE_2D, frontface_buffer);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA16F_ARB, WINDOW_SIZE, WINDOW_SIZE, 0, GL_RGBA, GL_FLOAT, NULL);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, frontface_buffer, 0);

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

	//compute the model dimensions
	nv::vec3f minPos(0,0,0), maxPos(1,1,1);
	center = (minPos + maxPos) * 0.5f;
	diameter = nv::length(maxPos - minPos);

	//manipulator.setDollyPosition(-1.5f * diameter);
	manipulator.setDollyPosition(-1 * diameter);
	manipulator.setDollyActivate(GLUT_LEFT_BUTTON, GLUT_ACTIVE_CTRL);
	//manipulator.setPanActivate(GLUT_LEFT_BUTTON, GLUT_ACTIVE_SHIFT);

	// read volume data file
	//read_volume_file(volume_filename);
	read_volume_file_with_tag(volume_filename);

	// init shaders
	setShaders();
}

/// the program's entry function
int main(int argc, char **argv)
{
	// print about information
	filename_utility::print_about(argc, argv);

	// get volume filename from arguments or console input
	filename_utility::get_filename(argc, argv, volume_filename);

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	//glutInitWindowPosition(100,100);
	glutInitWindowSize(WINDOW_SIZE,WINDOW_SIZE);

	char str[MAX_STR_SIZE];
	sprintf(str, "GPU raycasting - %s", volume_filename);
	glutCreateWindow(str);

	// keyboard
	glutKeyboardFunc(key_press);
	glutKeyboardUpFunc(key_release);
	glutSpecialFunc(special);

	// mouse
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(passiveMotion);

	//glutDisplayFunc(renderScene);
	//glutIdleFunc(renderScene);
	//glutReshapeFunc(changeSize);
	//glutKeyboardFunc(processNormalKeys);

	glutDisplayFunc(display);
	glutIdleFunc(idle_func);
	glutReshapeFunc(resize);
	resize(WINDOW_SIZE,WINDOW_SIZE);

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

	//setShaders();
	initialize();

	glutMainLoop();

	finailize();

	return 0;
}

