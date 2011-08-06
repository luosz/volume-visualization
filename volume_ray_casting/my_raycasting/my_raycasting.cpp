/**	@file
*	GPU raycasting with transfer functions and peeling
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

/**	@mainpage GPU Volume Raycasting by Shengzhou Luo (c) 2010-2011
*	GPU raycasting tutorial made by Peter Trier jan 2007
*	
*	This file contains all the elements nessesary to implement a simple 
*	GPU volume raycaster.
*	Notice this implementation requires a shader model 3.0 gfxcard.
*	
*	Adapted by Shengzhou Luo (ark) 2010-2011
*	Transfer functions, peeling, and importance driven rendering are written by Shengzhou Luo (ark)
*	
*	We implemented our approach with OpenGL and GLSL (OpenGL Shading Language) on a personal computer equipped with an AMD Athlon 7750 Dual-Core processor, 4GB memory and a NVIDIA GeForce GT 240 graphics card.
*	Experiments are conducted on several common datasets that are publicly available on the Volume Library. The original datasets in PVM format are converted into RAW format with the PVM tools distributed with the V^3 (Versatile Volume Viewer) volume rendering package.
*
*	Last updated: 2011-8-5
*/

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <cstdio>
#include <iostream>
#include <vector>
#include <limits>
#include <string>
using namespace std;

/// NVIDIA OpenGL SDK
#include <nvGlutManipulators.h>
#include <nvGlutWidgets.h>

#include "../BenBenRaycasting/transfer_function.h"
#include "VolumeReader.h"
#include "textfile.h"
#include "reader.h"
#include "volume_utility.h"
#include "filename_utility.h"

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
GLuint transfer_texture, transfer_texture2;
GLuint gradient_texture;

/// for fusion of two transfer functions
float fusion_factor = 0;
GLuint loc_fusion_factor;

/// histogram equalization in shaders
float scalar_min_normalized = 0;
GLuint loc_scalar_min_normalized;
float scalar_max_normalized = 1;
GLuint loc_scalar_max_normalized;

/// for clustering
GLuint cluster_texture;
GLuint loc_cluster_texture;

/// for importance peeling
GLuint importance_texture;
GLuint loc_importance_texture;

/// for feature peeling
float slope_threshold = 0;
GLuint loc_slope_threshold;
const float SLOPE_THRESHOLD_MAX = 1;
const float SLOPE_THRESHOLD_MIN = -1;
const float SLOPE_THRESHOLD_INC = 0.05;

/// for layer peeling
float peeling_layer = 0;
GLuint loc_peeling_layer;
const float LAYER_MAX = 100;
const float LAYER_MIN = 0;
const float LAYER_INC = 1;

/// for opacity peeling and gradient peeling
float threshold_low = 0.3;
float threshold_high = 0;
GLuint loc_threshold_high;
GLuint loc_threshold_low;
GLuint loc_peeling_option;

const float OPACITY_THRESHOLD_MAX = 1;
const float OPACITY_THRESHOLD_MIN = 0;
const float OPACITY_THRESHOLD_INC = 0.01;

const float GRADIENT_THRESHOLD_MAX = 50;
const float GRADIENT_THRESHOLD_MIN = 0;
const float GRADIENT_THRESHOLD_INC = 0.5;

const float GRADIENT_SAMPLE_THRESHOLD_MAX = 5;
const float GRADIENT_SAMPLE_THRESHOLD_MIN = 0;
const float GRADIENT_SAMPLE_THRESHOLD_INC = 0.05;

/// for linear interpolation of alpha in the transfer function
GLuint loc_alpha_opacity;
float alpha_opacity = 0;

/// the parameter k for k-means
float cluster_quantity = 8; // 16 clusters at most, since the cluster number is represented in 0~9 and a~f
int cluster_quantity_int_old = -1; // to be initialized
float cluster_interval;
GLuint loc_cluster_interval;

/// for shaders
const float STEPSIZE_MAX = 1.0/4.0;
const float STEPSIZE_MIN = 1e-4;
const float STEPSIZE_INC = STEPSIZE_MIN;
float stepsize = 1.0/100.0;

/// GLSL shaders
GLuint v,f,p;

GLuint loc_stepsize;
GLuint loc_volume_texture_from_file;
GLuint loc_transfer_texture, loc_transfer_texture2;
GLuint loc_gradient_texture;
const float LUMINANCE_MAX = 200;
const float LUMINANCE_MIN = 1;
const float LUMINANCE_INC = 1;
float luminance = 1;
GLuint loc_luminance;
GLuint loc_sizes;
const float CLIP_MAX = 1.732;
const float CLIP_MIN = 0;
const float CLIP_INC = 0.01;
float clip = 0;
GLuint loc_clip;

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
bool button_show_generated_cube = false;
bool button_show_generated_cube_backup = false;
bool button_auto_rotate = false;
bool button_lock_viewpoint = false;
bool button_generate_histogram = false;
bool button_cluster = false;
bool button_load_cluster_from_file = false;
bool button_generate_Ben_transfer_function = false;
bool button_generate_fusion_transfer_function = false;
bool button_show_alpha_blending = false;
bool button_load_importance_label = false;

// to enable or disable lighting
bool button_set_lighting_parameters = false;

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

/// for peeling
enum PeelingOption
{
	PEELING_NONE,
	PEELING_OPACITY,
	PEELING_FEATURE,
	PEELING_BACK,
	PEELING_FRONT,
	PEELING_GRADIENT,
	PEELING_OPACITY_IMPORTANCE,
	PEELING_GRADIENT_IMPORTANCE,
	PEELING_COUNT
};
int peeling_option = PEELING_NONE;

/// for transfer function
enum TransferFunctionOption
{
	TRANSFER_FUNCTION_NONE,
	TRANSFER_FUNCTION_2D,
	TRANSFER_FUNCTION_BEN,
	TRANSFER_FUNCTION_GRADIENTS_AS_COLORS,
	TRANSFER_FUNCTION_2ND_DERIVATIVE,
	TRANSFER_FUNCTION_SOBEL,
	TRANSFER_FUNCTION_SOBEL_3D,
	TRANSFER_FUNCTION_SOBEL_3D_TEXTURE,
	TRANSFER_FUNCTION_K_MEANS,
	TRANSFER_FUNCTION_K_MEANS_EQUALIZED,
	TRANSFER_FUNCTION_K_MEANS_IMPORTANCE,
	TRANSFER_FUNCTION_K_MEANS_EQUALIZED_IMPORTANCE,
	TRANSFER_FUNCTION_SOBEL_3D_IMPORTANCE,
	TRANSFER_FUNCTION_FUSION,
	TRANSFER_FUNCTION_COUNT
};
int transfer_function_option = TRANSFER_FUNCTION_NONE;
GLuint loc_transfer_function_option;

enum LightingOption
{
	LIGHTING_DISABLE,
	LIGHTING_ENABLE,
	LIGHTING_COUNT
};
int lighting_option = LIGHTING_ENABLE;
GLuint loc_lighting_option;

// for lighting
float fSpecularPower = 25;
float fvLightPosition[3] = {-100, 100, 100};
float fvEyePosition[3] = {0, 0, 100};
//float fvAmbient[4] = {0.368627, 0.368421, 0.368421, 1.0};
//float fvDiffuse[4] = {0.490196, 0.488722, 0.488722, 1.0};
//float fvSpecular[4] = {0.886275, 0.885003, 0.885003, 1.0};
float fvAmbient[4] = {0.768627, 0.768421, 0.568421, 1.0};
float fvDiffuse[4] = {0.790196, 0.988722, 0.988722, 1.0};
float fvSpecular[4] = {0.986275, 0.985003, 0.985003, 1.0};
GLuint loc_fSpecularPower;
GLuint loc_fvLightPosition;
GLuint loc_fvEyePosition;
GLuint loc_fvAmbient;
GLuint loc_fvSpecular;
GLuint loc_fvDiffuse;

// set the lighting parameters
void set_lighting_parameters()
{
	std::cout<<"Lighting parameters:"<<std::endl;
	std::cout<<"Specular power "<<fSpecularPower<<std::endl;
	std::cout<<"Light position "<<fvLightPosition[0]<<" "<<fvLightPosition[1]<<" "<<fvLightPosition[2]<<std::endl;
	std::cout<<"Eye position "<<fvEyePosition[0]<<" "<<fvEyePosition[1]<<" "<<fvEyePosition[2]<<std::endl;
	std::cout<<"Ambient "<<fvAmbient[0]<<" "<<fvAmbient[1]<<" "<<fvAmbient[2]<<" "<<fvAmbient[3]<<std::endl;
	std::cout<<"Diffuse "<<fvDiffuse[0]<<" "<<fvDiffuse[1]<<" "<<fvDiffuse[2]<<" "<<fvDiffuse[3]<<std::endl;
	std::cout<<"Specular "<<fvSpecular[0]<<" "<<fvSpecular[1]<<" "<<fvSpecular[2]<<" "<<fvSpecular[3]<<std::endl;
	std::cout<<"\nWould you like to set new values for the parameters? y/n"<<std::endl;
	string s;
	std::cin>>s;
	if (s.compare("y") == 0 || s.compare("Y") == 0)
	{
		std::cout<<"Specular power (e.g. 25)"<<std::endl;
		std::cin>>fSpecularPower;
		std::cout<<"Light position (e.g. -100 100 100)"<<std::endl;
		std::cin>>fvLightPosition[0]>>fvLightPosition[1]>>fvLightPosition[2];
		std::cout<<"Eye position (e.g. 0 0 100)"<<std::endl;
		std::cin>>fvEyePosition[0]>>fvEyePosition[1]>>fvEyePosition[2];
		std::cout<<"Ambient (e.g. 0.368627 0.368421 0.368421 1.0)"<<std::endl;
		std::cin>>fvAmbient[0]>>fvAmbient[1]>>fvAmbient[2]>>fvAmbient[3];
		std::cout<<"Diffuse (e.g. 0.490196 0.488722 0.488722 1.0)"<<std::endl;
		std::cin>>fvDiffuse[0]>>fvDiffuse[1]>>fvDiffuse[2]>>fvDiffuse[3];
		std::cout<<"Specular (e.g. 0.886275 0.885003 0.885003 1.0)"<<std::endl;
		std::cin>>fvSpecular[0]>>fvSpecular[1]>>fvSpecular[2]>>fvSpecular[3];
	}
	std::cout<<"Done."<<std::endl;
}

/************************************************************************/
/// for ui widgets
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

/// the value picked on the scalar histogram
float picked = 0.5;

/// draw ui widgets
void doUI()
{
	nv::Rect none;
	const char *render_str[RENDER_COUNT] = {"Final image", "Back faces", "Front faces", "2D transfer function", "Histogram", "Gradient"};
	const char *peeling_str[PEELING_COUNT] = {"No peeling", "Opacity peeling", "Feature peeling", "Peel back layers", "Peel front layers", "Gradient peeling", "Opacity with importance", "Gradient with importance"};
	const char *transfer_function_str[TRANSFER_FUNCTION_COUNT] = {"No transfer function", "2D", "Ben", "Gradients as colors", "2nd derivative", "Sobel", "Sobel 3D", "Sobel 3D texture", "K-means++", "K-means++ equalized", "2D importance", "K-means++ importance", "Sobel 3D importance", "Fusion"};
	const char *lighting_str[LIGHTING_COUNT] = {"Disable lighting", "Enable lighting"};

	glDisable(GL_CULL_FACE);

	ui.begin();

	if (ui_on)
	{
		ui.beginGroup();

		ui.beginGroup(nv::GroupFlags_GrowRightFromBottom|nv::GroupFlags_LayoutNoMargin);
		//ui.doCheckButton(none, "Test cube", &button_show_generated_cube);
		ui.doCheckButton(none, "Rotate", &button_auto_rotate);
		ui.doCheckButton(none, "View lock", &button_lock_viewpoint);
		ui.doCheckButton(none, "Alpha blend", &button_show_alpha_blending);
		ui.doButton(none, "Histogram", &button_generate_histogram);
		ui.doButton(none, "Cluster", &button_cluster);
		ui.doButton(none, "from file", &button_load_cluster_from_file);
		ui.doButton(none, "Ben TF", &button_generate_Ben_transfer_function);
		ui.doButton(none, "Fusion", &button_generate_fusion_transfer_function);
		ui.doButton(none, "Importance", &button_load_importance_label);
		ui.doButton(none, "Lighting", &button_set_lighting_parameters);
		ui.endGroup();

		ui.doComboBox(none, RENDER_COUNT, render_str, &render_option);
		ui.doComboBox(none, PEELING_COUNT, peeling_str, &peeling_option);
		ui.doComboBox(none, TRANSFER_FUNCTION_COUNT, transfer_function_str, &transfer_function_option);

		ui.doComboBox(none, LIGHTING_COUNT, lighting_str, &lighting_option);

		//ui.doLineEdit(none, text, MAX_STR_SIZE, &chars_returned);

		ui.endGroup();

		ui.beginGroup(nv::GroupFlags_GrowDownFromRight);
		char str[MAX_STR_SIZE];
		sprintf(str, "Step size: %f", stepsize);
		ui.doLabel(none, str);

		nv::Rect rect_slider(0,0,600,0);
		ui.doHorizontalSlider(rect_slider, STEPSIZE_MIN, STEPSIZE_MAX, &stepsize);

		// show peeling widgets
		switch(peeling_option)
		{
		case PEELING_OPACITY_IMPORTANCE:
		case PEELING_OPACITY:
			// if(accumulated>high && sampled<low)
			sprintf(str, "peeling condition: accumulated>high && sampled<low    threshold low: %f threshold high: %f", threshold_low, threshold_high);
			ui.doLabel(none, str);
			ui.doHorizontalSlider(rect_slider, OPACITY_THRESHOLD_MIN, OPACITY_THRESHOLD_MAX, &threshold_low);
			ui.doHorizontalSlider(rect_slider, OPACITY_THRESHOLD_MIN, OPACITY_THRESHOLD_MAX, &threshold_high);
			break;
		case PEELING_FEATURE:
			sprintf(str, "slope threshold: %f", slope_threshold);
			ui.doLabel(none, str);
			ui.doHorizontalSlider(rect_slider, SLOPE_THRESHOLD_MIN, SLOPE_THRESHOLD_MAX, &slope_threshold);
			break;
		case PEELING_GRADIENT_IMPORTANCE:
		case PEELING_GRADIENT:
			// if(accumulated>high && sampled<low)
			sprintf(str, "peeling condition: accumulated>high && sampled<low    threshold low: %f threshold high: %f", threshold_low, threshold_high);
			ui.doLabel(none, str);
			ui.doHorizontalSlider(rect_slider, GRADIENT_THRESHOLD_MIN, GRADIENT_THRESHOLD_MAX, &threshold_low);
			ui.doHorizontalSlider(rect_slider, GRADIENT_THRESHOLD_MIN, GRADIENT_THRESHOLD_MAX, &threshold_high);
			break;
		}
		switch(peeling_option)
		{
		case PEELING_OPACITY:
		case PEELING_FEATURE:
		case PEELING_BACK:
		case PEELING_FRONT:
		case PEELING_GRADIENT:
		case PEELING_OPACITY_IMPORTANCE:
		case PEELING_GRADIENT_IMPORTANCE:
			sprintf(str, "peeling_layer: %f", peeling_layer);
			ui.doLabel(none, str);
			ui.doHorizontalSlider(rect_slider, LAYER_MIN, LAYER_MAX, &peeling_layer);
			break;
		}

		// show transfer function widgets
		if (transfer_function_option == TRANSFER_FUNCTION_K_MEANS || transfer_function_option == TRANSFER_FUNCTION_K_MEANS_EQUALIZED)
		{
			sprintf(str, "k-means k=%f", cluster_quantity);
			ui.doLabel(none, str);
			ui.doHorizontalSlider(rect_slider, 1, 16, &cluster_quantity);
		}else
		{
			if (button_show_alpha_blending && (transfer_function_option == TRANSFER_FUNCTION_SOBEL || transfer_function_option == TRANSFER_FUNCTION_SOBEL_3D))
			{
				sprintf(str, "opacity=mix(gradient,scalar,alpha)    alpha=%f", alpha_opacity);
				ui.doLabel(none, str);
				ui.doHorizontalSlider(rect_slider, 0, 1, &alpha_opacity);
			}else
			{
				if (transfer_function_option == TRANSFER_FUNCTION_FUSION)
				{
					sprintf(str, "fusion factor = %f", fusion_factor);
					ui.doLabel(none, str);
					ui.doHorizontalSlider(rect_slider, 0, 1, &fusion_factor);
				}
			}
		}

		ui.endGroup();

		ui.beginGroup(nv::GroupFlags_GrowUpFromLeft);
		nv::Rect full_slider(-5,0,800,0);
		ui.doHorizontalSlider(full_slider, LUMINANCE_MIN, LUMINANCE_MAX, &luminance);
		sprintf(str, "Luminance: %f", luminance);
		ui.doLabel(none, str);
		ui.doHorizontalSlider(full_slider, CLIP_MIN, CLIP_MAX, &clip);
		sprintf(str, "Clip: %f", clip);
		ui.doLabel(none, str);
		if (render_option == RENDER_HISTOGRAM)
		{
			ui.doHorizontalSlider(full_slider, 0.00001, 1.0, &picked);
			sprintf(str, "Histogram value picked: %f", picked);
			ui.doLabel(none, str);
		}
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

/************************************************************************/
/// Shader initilization
/************************************************************************/
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
void setShaders() {

	char *vs = NULL, *fs = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);

	vs = file_reader::textFileRead("simple_vertex.vert.cc");
	fs = file_reader::textFileRead("my_raycasting.frag.cc");

	const char * vv = vs;
	const char * ff = fs;

	glShaderSource(v, 1, &vv,NULL);
	glShaderSource(f, 1, &ff,NULL);

	free(vs);free(fs);

	glCompileShader(v);
	glCompileShader(f);

	printShaderInfoLog(v);
	printShaderInfoLog(f);

	p = glCreateProgram();
	glAttachShader(p,v);
	glAttachShader(p,f);

	// Initial program setup.
	glLinkProgram(p); // Initial link

	glUseProgram(p);
	loc_stepsize = glGetUniformLocation(p, "stepsize");
	loc_threshold_high = glGetUniformLocation(p, "threshold_high");
	loc_threshold_low = glGetUniformLocation(p, "threshold_low");
	loc_peeling_option = glGetUniformLocation(p, "peeling_option");
	loc_transfer_function_option = glGetUniformLocation(p, "transfer_function_option");
	loc_luminance = glGetUniformLocation(p, "luminance");
	loc_sizes = glGetUniformLocation(p, "sizes");
	loc_clip = glGetUniformLocation(p, "clip");
	loc_slope_threshold = glGetUniformLocation(p, "slope_threshold");
	loc_cluster_interval = glGetUniformLocation(p, "cluster_interval");
	loc_peeling_layer = glGetUniformLocation(p, "peeling_layer");
	loc_scalar_min_normalized = glGetUniformLocation(p, "scalar_min_normalized");
	loc_scalar_max_normalized = glGetUniformLocation(p, "scalar_max_normalized");
	loc_alpha_opacity = glGetUniformLocation(p, "alpha_opacity");
	loc_fusion_factor = glGetUniformLocation(p, "fusion_factor");

	// for lighting
	loc_fSpecularPower = glGetUniformLocation(p, "fSpecularPower");
	loc_fvLightPosition = glGetUniformLocation(p, "fvLightPosition");
	loc_fvEyePosition = glGetUniformLocation(p, "fvEyePosition");
	loc_fvAmbient = glGetUniformLocation(p, "fvAmbient");
	loc_fvSpecular = glGetUniformLocation(p, "fvSpecular");
	loc_fvDiffuse = glGetUniformLocation(p, "fvDiffuse");
	loc_lighting_option = glGetUniformLocation(p, "lighting_option");

	// set textures
	add_texture_uniform(p, "front", 1, GL_TEXTURE_2D, frontface_buffer);
	add_texture_uniform(p, "back", 2, GL_TEXTURE_2D, backface_buffer);
	loc_volume_texture_from_file = add_texture_uniform(p, "volume_texture", 3, GL_TEXTURE_3D, volume_texture_from_file);
	add_texture_uniform(p, "transfer_function_2D", 4, GL_TEXTURE_2D, transfer_function_2D_buffer);
	loc_transfer_texture = add_texture_uniform(p, "transfer_texture", 5, GL_TEXTURE_3D, transfer_texture);
	loc_cluster_texture =  add_texture_uniform(p, "cluster_texture", 6, GL_TEXTURE_3D, cluster_texture);
	loc_importance_texture =  add_texture_uniform(p, "importance_texture", 7, GL_TEXTURE_3D, importance_texture);
	loc_transfer_texture2 = add_texture_uniform(p, "transfer_texture2", 8, GL_TEXTURE_3D, transfer_texture2);

	// this texture is for saving the precomputed gradients
	loc_gradient_texture =  add_texture_uniform(p, "gradient_texture", 9, GL_TEXTURE_3D, gradient_texture);

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

/// create a test volume texture, here you could load your own volume
void create_volumetexture_a_cube()
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

/// create a funsion transfer function
void create_transferfunc_fusion()
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
	glTexImage3D(GL_TEXTURE_3D, 0,GL_RGBA, volume.getX(), volume.getY(), volume.getZ(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tf);

	// free the transfer function pointer after texture mapping
	free_transfer_function_pointer(tf);

	color_opacity * tf2 = NULL;
	setTransferfunc6(tf2, volume);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &transfer_texture2);
	glBindTexture(GL_TEXTURE_3D, transfer_texture2);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0,GL_RGBA, volume.getX(), volume.getY(), volume.getZ(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tf2);

	// free the transfer function pointer after texture mapping
	free_transfer_function_pointer(tf2);
}

/// call this function when reshape
void reshape_ortho(int w, int h)
{
	if (h == 0) h = 1;
	glViewport(0, 0,w, h);
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

/// load the importance labels as a texture
void load_importance_label_texture(unsigned char *label_ptr, GLuint location, GLuint program, const char* name, int number, GLuint texture)
{
	GLenum target = GL_TEXTURE_3D;

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &texture);
	glBindTexture(target, texture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexImage3D(target, 0, 1, sizes[0], sizes[1], sizes[2], 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, label_ptr);

	set_texture_uniform(location, program, name, number, target, texture);
}

/// read importance labels from command line
void load_importance_label(const unsigned int count)
{
	unsigned char *label_ptr_replaced = new unsigned char[count];
	unsigned char *label_ptr = new unsigned char[count];
	int k = static_cast<int>(cluster_quantity);
	unsigned char *replacement = new unsigned char[k];

	// get the filename of cluster labels
	char label_filename[MAX_STR_SIZE];
	sprintf(label_filename, "%s.%d.txt", volume_filename, k);

	// get the filename of replaced labels
	char label_filename_replaced[MAX_STR_SIZE];
	sprintf(label_filename_replaced, "%s.%d.replaced.txt", volume_filename, k);

	std::cout<<"Input a replacement list (e.g. 01234567 for k=8, 0123456789abcdef for k=16)"<<std::endl;
	for (int i=0; i<k; i++)
	{
		std::cin>>replacement[i];
		replacement[i] = volume_utility::char_to_number(replacement[i]);
	}

	// read labels from file
	std::cout<<"Read labels from file "<<label_filename<<std::endl;
	std::ifstream label_file(label_filename);
	if (label_file.bad())
	{
		std::cout<<"Failed to open file "<<label_filename<<std::endl;
		return;
	}
	for (unsigned int i=0; i<count; i++)
	{
		label_file>>label_ptr[i];
		label_ptr[i] = volume_utility::char_to_number(label_ptr[i]);
		label_ptr_replaced[i] = replacement[label_ptr[i]];
	}
	label_file.close();

	// write replaced labels to file
	std::cout<<"Write replaced labels to file "<<label_filename_replaced<<std::endl;
	std::ofstream label_file_replaced(label_filename_replaced);
	if (label_file_replaced.bad())
	{
		std::cout<<"Failed to open file "<<label_filename_replaced<<std::endl;
		return;
	}
	for (unsigned int i=0; i<count; i++)
	{
		label_file_replaced<<std::hex<<(int)label_ptr_replaced[i];
	}
	label_file_replaced.close();

	// shift labels from range 0..7 or 0..f to 0..255
	std::cout<<"Shifting labels..."<<std::endl;
	volume_utility::shift_labels(k, count, label_ptr);
	volume_utility::shift_labels(k, count, label_ptr_replaced);

	// load the labels as texture
	load_importance_label_texture(label_ptr, loc_cluster_texture, p, "cluster_texture", 6, cluster_texture);
	load_importance_label_texture(label_ptr_replaced, loc_importance_texture, p, "importance_texture", 7, importance_texture);

	std::cout<<"Importance labels are loaded"<<std::endl<<std::endl;

	delete [] label_ptr_replaced;
	delete [] label_ptr;
}

/// cluster the volume data
template <class T, int TYPE_SIZE>
void cluster(const T *data, const unsigned int count)
{
	unsigned char *label_ptr = new unsigned char[count];
	int k = static_cast<int>(cluster_quantity);

	volume_utility::cluster<T, TYPE_SIZE>(data, count, (unsigned int)color_component_number, k, label_ptr, sizes[0], sizes[1], sizes[2]);

	char label_filename[MAX_STR_SIZE];
	sprintf(label_filename, "%s.%d.txt", volume_filename, k);

	std::cout<<"Write labels to file "<<label_filename<<std::endl;
	std::ofstream label_file(label_filename);
	for (unsigned int i=0; i<count; i++)
	{
		label_file<<std::hex<<(int)label_ptr[i];
	}
	label_file.close();

	// shift labels from range 0..7 or 0..f to 0..255
	std::cout<<"Shifting labels..."<<std::endl;
	volume_utility::shift_labels(k, count, label_ptr);

	// load the labels as texture
	load_importance_label_texture(label_ptr, loc_cluster_texture, p, "cluster_texture", 6, cluster_texture);
	load_importance_label_texture(label_ptr, loc_importance_texture, p, "importance_texture", 7, importance_texture);

	std::cout<<"Clustering finished."<<std::endl;

	delete [] label_ptr;
}

/// read cluster labels from file
void load_cluster_labels_from_file(const unsigned int count)
{
	unsigned char *label_ptr = new unsigned char[count];
	int k = static_cast<int>(cluster_quantity);

	// get the filename of cluster labels
	char label_filename[MAX_STR_SIZE];
	sprintf(label_filename, "%s.%d.txt", volume_filename, k);

	std::cout<<"Read labels from file "<<label_filename<<std::endl;
	std::ifstream label_file(label_filename);
	if (label_file.bad())
	{
		std::cout<<"Failed to open file "<<label_filename<<std::endl;
		return;
	}
	for (unsigned int i=0; i<count; i++)
	{
		label_file>>label_ptr[i];
		label_ptr[i] = volume_utility::char_to_number(label_ptr[i]);
	}
	label_file.close();

	// shift labels from range 0..7 or 0..f to 0..255
	std::cout<<"Shifting labels..."<<std::endl;
	volume_utility::shift_labels(k, count, label_ptr);

	// load the labels as texture
	load_importance_label_texture(label_ptr, loc_cluster_texture, p, "cluster_texture", 6, cluster_texture);
	load_importance_label_texture(label_ptr, loc_importance_texture, p, "importance_texture", 7, importance_texture);

	std::cout<<"Cluster labels loaded from file."<<std::endl;

	delete [] label_ptr;
}

/// render the histogram
template <class T, int TYPE_SIZE>
void render_histograms(const T *data, const unsigned int count, const unsigned int components)
{
	unsigned int histogram[TYPE_SIZE] = {0};
	vector<float> scalar_value(count); // the scalar data in const T *data
	vector<nv::vec3f> gradient(count);
	vector<float> gradient_magnitude(count);
	vector<nv::vec3f> second_derivative(count);
	vector<float> second_derivative_magnitude(count);
	float max_gradient_magnitude, max_second_derivative_magnitude;
	volume_utility::generate_scalar_histogram<T, TYPE_SIZE>(data, count, components, histogram, scalar_value);
	volume_utility::find_min_max_scalar_in_histogram<T, TYPE_SIZE>(count, histogram, scalar_min_normalized, scalar_max_normalized);
	volume_utility::generate_gradient(sizes, count, components, scalar_value, gradient, gradient_magnitude, max_gradient_magnitude, second_derivative, second_derivative_magnitude, max_second_derivative_magnitude);

	// draw scalar histogram
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, histogram_buffer, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glLoadIdentity();
	reshape_ortho(WINDOW_SIZE,WINDOW_SIZE); // set projection mode to Ortho 2D
	glDisable(GL_DEPTH_TEST);

	// draw quadrangle strips to make a histogram
	glBegin(GL_QUAD_STRIP);
	float x, y, height = (1/(1-0.618)) * float(count) / TYPE_SIZE;
	for (unsigned int i = 0; i<TYPE_SIZE; i++)
	{
		x = float(i) / TYPE_SIZE;
		y = float(histogram[i]) / height;
		glColor3f(x, x, x);
		glColor3f(1.0, 1.0, 1.0);
		glVertex2f(x, 0);
		glColor3f(1.0, 1.0, 1.0);
		glVertex2f(x, y);
	}
	glEnd();

	// draw gradient histogram
	if (max_gradient_magnitude > 0)
	{
		glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, histogram_gradient_buffer, 0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
		glLoadIdentity();
		glEnable(GL_POINT_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// draw points to make a gradient histogram
		glBegin(GL_POINTS);
		for (unsigned int i = 0; i<count; i++)
		{
			x = scalar_value[i] / TYPE_SIZE;
			y = gradient_magnitude[i] / max_gradient_magnitude;
			glColor3f(abs(gradient[i].x/max_gradient_magnitude), abs(gradient[i].y/max_gradient_magnitude), abs(gradient[i].z/max_gradient_magnitude));
			glVertex2f(x, y);
		}
		glEnd();
		glDisable(GL_BLEND);
	}

	glEnable(GL_DEPTH_TEST);
	resize(WINDOW_SIZE,WINDOW_SIZE); // set projection mode back to 3D
}

/// read volume data from file
void read_volume_file(char* filename) 
{
	float dists[3];
	file_reader::DataType type;

	if (!data_ptr)
	{
		data_ptr = new void *;
	}
	file_reader::readData(filename, sizes, dists, data_ptr, &type, &color_component_number);

	switch (type)
	{
	case file_reader::DATRAW_UCHAR:
		gl_type = GL_UNSIGNED_BYTE;
		break;
	case file_reader::DATRAW_USHORT:
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

/// estimate gradient texture
void load_gradient_texture()
{
	std::cout<<"Estimate gradient texture..."<<std::endl;

	unsigned int count = sizes[0]*sizes[1]*sizes[2];
	unsigned short *gradient_data = new unsigned short[count * 3];

	vector<float> scalar_value(count); // the scalar data in const T *data
	vector<nv::vec3f> gradient(count);
	std::cout<<"Scalar histogram..."<<std::endl;

	if (gl_type == GL_UNSIGNED_SHORT)
	{
		unsigned int histogram[65536] = {0};
		volume_utility::generate_scalar_histogram<unsigned short, 65536>((unsigned short*)*data_ptr, count, (unsigned int)color_component_number, histogram, scalar_value);
	}
	else
	{
		unsigned int histogram[256] = {0};
		volume_utility::generate_scalar_histogram<unsigned char, 256>((unsigned char*)*data_ptr, count, (unsigned int)color_component_number, histogram, scalar_value);
	}

	volume_utility::estimate_gradient(gradient_data, sizes, count, scalar_value, gradient);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &gradient_texture);
	glBindTexture(GL_TEXTURE_3D, gradient_texture);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_BORDER);
	glTexImage3D(GL_TEXTURE_3D, 0, 3, sizes[0], sizes[1], sizes[2], 0, GL_RGB, GL_UNSIGNED_SHORT, gradient_data);

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

	create_volumetexture_a_cube();

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
	read_volume_file(volume_filename);

	// estimate gradients for voxels and load them as a texture
	load_gradient_texture();

	// init shaders
	setShaders();
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
		case 'c':
			clip = increase(clip, CLIP_INC, CLIP_MAX);
			break;
		case 'x':
			clip = decrease(clip, CLIP_INC, CLIP_MIN);
			break;
		case 'h':
			switch(peeling_option)
			{
			case PEELING_OPACITY_IMPORTANCE:
			case PEELING_OPACITY:
				threshold_low = decrease(threshold_low, OPACITY_THRESHOLD_INC, OPACITY_THRESHOLD_MIN);
				break;
			case PEELING_GRADIENT_IMPORTANCE:
			case PEELING_GRADIENT:
				threshold_low = decrease(threshold_low, GRADIENT_SAMPLE_THRESHOLD_INC, GRADIENT_SAMPLE_THRESHOLD_MIN);
				break;
			case PEELING_FEATURE:
				slope_threshold = decrease(slope_threshold, SLOPE_THRESHOLD_INC, SLOPE_THRESHOLD_MIN);
				break;
			}
			break;
		case 'j':
			switch(peeling_option)
			{
			case PEELING_OPACITY_IMPORTANCE:
			case PEELING_OPACITY:
				threshold_low = increase(threshold_low, OPACITY_THRESHOLD_INC, OPACITY_THRESHOLD_MAX);
				break;
			case PEELING_GRADIENT_IMPORTANCE:
			case PEELING_GRADIENT:
				threshold_low = increase(threshold_low, GRADIENT_SAMPLE_THRESHOLD_INC, GRADIENT_SAMPLE_THRESHOLD_MAX);
				break;
			case PEELING_FEATURE:
				slope_threshold = increase(slope_threshold, SLOPE_THRESHOLD_INC, SLOPE_THRESHOLD_MAX);
				break;
			}
			break;
		case 'n':
			switch(peeling_option)
			{
			case PEELING_OPACITY_IMPORTANCE:
			case PEELING_OPACITY:
				threshold_high = decrease(threshold_high, OPACITY_THRESHOLD_INC, OPACITY_THRESHOLD_MIN);
				break;
			case PEELING_GRADIENT_IMPORTANCE:
			case PEELING_GRADIENT:
				threshold_high = decrease(threshold_high, GRADIENT_THRESHOLD_INC, GRADIENT_THRESHOLD_MIN);
				break;
			}
			break;
		case 'm':
			switch(peeling_option)
			{
			case PEELING_OPACITY_IMPORTANCE:
			case PEELING_OPACITY:
				threshold_high = increase(threshold_high, OPACITY_THRESHOLD_INC, OPACITY_THRESHOLD_MAX);
				break;
			case PEELING_GRADIENT_IMPORTANCE:
			case PEELING_GRADIENT:
				threshold_high = increase(threshold_high, GRADIENT_THRESHOLD_INC, GRADIENT_THRESHOLD_MAX);
				break;
			}
			break;
		case 'k':
			switch(peeling_option)
			{
			case PEELING_OPACITY:
			case PEELING_FEATURE:
			case PEELING_BACK:
			case PEELING_FRONT:
			case PEELING_GRADIENT:
			case PEELING_OPACITY_IMPORTANCE:
			case PEELING_GRADIENT_IMPORTANCE:
				peeling_layer = decrease(peeling_layer, LAYER_INC, LAYER_MIN);
				break;
			}
			break;
		case 'l':
			switch(peeling_option)
			{
			case PEELING_OPACITY:
			case PEELING_FEATURE:
			case PEELING_BACK:
			case PEELING_FRONT:
			case PEELING_GRADIENT:
			case PEELING_OPACITY_IMPORTANCE:
			case PEELING_GRADIENT_IMPORTANCE:
				peeling_layer = increase(peeling_layer, LAYER_INC, LAYER_MAX);
				break;
			}
			break;
		}
	}
}

/// GLUT callback function, trigger when keys are pressed
void key_press(unsigned char key, int x, int y)
{
	gKeys[key] = true;
}

/// GLUT callback function, trigger when keys are released
void key_release(unsigned char key, int x, int y)
{
	gKeys[key] = false;
	switch (key)
	{
		//case 27 :
		//	// escape to exit
		//	exit(0);
		//	break;
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
	case 'p':
		// peeling
		if (glutGetModifiers() == GLUT_ACTIVE_ALT)
		{
			peeling_option = (peeling_option - 1 + PEELING_COUNT) % PEELING_COUNT;
		}else
		{
			peeling_option = (peeling_option + 1) % PEELING_COUNT;
		}
		break;
	case 'x':
		// lighting options
		if (glutGetModifiers() == GLUT_ACTIVE_ALT)
		{
			lighting_option = (lighting_option - 1 + LIGHTING_COUNT) % LIGHTING_COUNT;
		}else
		{
			lighting_option = (lighting_option + 1) % LIGHTING_COUNT;
		}
		break;
		//case 'c':
		//	std::cout<<chars_returned<<"\t"<<text<<std::endl;
		//	break;
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
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
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

/// a raycasting pass
void raycasting_pass()
{
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, final_image, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	int peeling_layer_int = static_cast<int>(peeling_layer);
	peeling_layer = peeling_layer_int;

	int k = static_cast<int>(cluster_quantity);
	cluster_quantity = k;
	if (cluster_quantity_int_old != k)
	{
		cluster_quantity_int_old = k;
		cluster_interval = volume_utility::get_cluster_interval(cluster_quantity_int_old);
	}

	// Use shaders
	glUseProgram(p);
	glUniform1f(loc_stepsize, stepsize);
	glUniform1f(loc_threshold_high, threshold_high);
	glUniform1f(loc_threshold_low, threshold_low);
	glUniform1f(loc_luminance, luminance);
	glUniform1f(loc_clip, clip);
	glUniform3f(loc_sizes, sizes[0], sizes[1], sizes[2]);
	glUniform1f(loc_cluster_interval, cluster_interval);
	glUniform1f(loc_scalar_min_normalized, scalar_min_normalized);
	glUniform1f(loc_scalar_max_normalized, scalar_max_normalized);
	glUniform1f(loc_slope_threshold, slope_threshold);
	glUniform1f(loc_alpha_opacity, alpha_opacity);
	glUniform1f(loc_fusion_factor, fusion_factor);

	glUniform1i(loc_peeling_option, peeling_option);
	glUniform1i(loc_transfer_function_option, transfer_function_option);
	glUniform1i(loc_peeling_layer, peeling_layer_int);

	// for lighting
	glUniform1f(loc_fSpecularPower, fSpecularPower);
	glUniform3fv(loc_fvLightPosition, 1, fvLightPosition);
	glUniform3fv(loc_fvEyePosition, 1, fvEyePosition);
	glUniform4fv(loc_fvAmbient, 1, fvAmbient);
	glUniform4fv(loc_fvSpecular, 1, fvSpecular);
	glUniform4fv(loc_fvDiffuse, 1, fvDiffuse);
	glUniform1i(loc_lighting_option, lighting_option);

	if(button_generate_Ben_transfer_function)
	{
		button_generate_Ben_transfer_function = false;
		create_transferfunc_Ben();
		set_texture_uniform(loc_transfer_texture, p, "transfer_texture", 5, GL_TEXTURE_3D, transfer_texture);
	}

	if(button_generate_fusion_transfer_function)
	{
		button_generate_fusion_transfer_function = false;
		create_transferfunc_fusion();
		set_texture_uniform(loc_transfer_texture, p, "transfer_texture", 5, GL_TEXTURE_3D, transfer_texture);
		set_texture_uniform(loc_transfer_texture2, p, "transfer_texture2", 8, GL_TEXTURE_3D, transfer_texture2);
	}

	if(button_show_generated_cube != button_show_generated_cube_backup)
	{
		button_show_generated_cube_backup = button_show_generated_cube;
		if(button_show_generated_cube)
			set_texture_uniform(loc_volume_texture_from_file, p, "volume", 3, GL_TEXTURE_3D, volume_texture);
		else
			set_texture_uniform(loc_volume_texture_from_file, p, "volume", 3, GL_TEXTURE_3D, volume_texture_from_file);
	}

	// set lighting parameters
	if (button_set_lighting_parameters)
	{
		button_set_lighting_parameters = false;
		set_lighting_parameters();
	}

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

	// render the histogram
	if(data_ptr && button_generate_histogram)
	{
		button_generate_histogram = false;
		unsigned int count = sizes[0] * sizes[1] * sizes[2];
		switch(gl_type)
		{
		case GL_UNSIGNED_BYTE:
			render_histograms<unsigned char, 256>((unsigned char*)*data_ptr, count, color_component_number);
			break;
		case GL_UNSIGNED_SHORT:
			render_histograms<unsigned short, 65536>((unsigned short*)*data_ptr, count, color_component_number);
			break;
		default:
			std::cerr<<"Unsupported data type in the volume data."<<endl;
		}
	}

	// cluster
	if (data_ptr && button_cluster)
	{
		button_cluster = false;
		unsigned int count = sizes[0]*sizes[1]*sizes[2];
		if (gl_type == GL_UNSIGNED_SHORT)
			cluster<unsigned short, 65536>((unsigned short*)*data_ptr, count);
		else
			cluster<unsigned char, 256>((unsigned char*)*data_ptr, count);
	}

	// read cluster labels from file
	if (data_ptr && button_load_cluster_from_file)
	{
		button_load_cluster_from_file = false;
		unsigned int count = sizes[0]*sizes[1]*sizes[2];
		load_cluster_labels_from_file(count);
	}

	// load importance label
	if(data_ptr && button_load_importance_label)
	{
		button_load_importance_label = false;
		load_importance_label(sizes[0]*sizes[1]*sizes[2]);
	}

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

/// the program's entry function
int main(int argc, char* argv[])
{
	// print about information
	filename_utility::print_about(argc, argv);

	// get volume filename from arguments or console input
	filename_utility::get_filename(argc, argv, volume_filename);

	glutInit(&argc,argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);

	char str[MAX_STR_SIZE];
	sprintf(str, "GPU raycasting - %s", volume_filename);
	glutCreateWindow(str);
	glutReshapeWindow(WINDOW_SIZE,WINDOW_SIZE);

	// keyboard
	glutKeyboardFunc(key_press);
	glutKeyboardUpFunc(key_release);
	glutSpecialFunc(special);

	// mouse
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(passiveMotion);

	glutDisplayFunc(display);
	glutIdleFunc(idle_func);
	glutReshapeFunc(resize);
	resize(WINDOW_SIZE,WINDOW_SIZE);

	initialize();
	glutMainLoop();
	finailize();

	return 0;
}
