#ifndef  _LH_HISTOGRAMS_CONSTRUCTOR_
#define  _LH_HISTOGRAMS_CONSTRUCTOR_

#include "VolumeReader.h"
#include <vector>
#include <utility>
#include <nvMath.h>
using namespace std;
using namespace nv;

class LH_Histograms{
public:

	//epsilon_min for determine whether the gradient magnitude is small enough
	float m_epsilon_min;

	//epsilon_max for determine whether the difference between the gradient and second derivative is big enough
	float m_epsilon_max;

	//integration step for the second order Runge-Kutta method
	int m_integrationStep;

	//pointer to the VolumeReader object
	VolumeReader * m_volumeReader;

	//vector for store the FL,FH information of every voxel in the volume
	vector<pair<vec3f,pair<float,float>>> m_LHInformation;
	
public:

	//constructor and destructor
	LH_Histograms(VolumeReader *volume);
	~LH_Histograms();

	//main procedure for calculate the FL,FH for every voxel
	void constructor(vector<nv::vec3f> &gradient, vector<float> &gradient_magnitude, vector<nv::vec3f> &second_derivative, vector<float> &second_derivative_magnitude);

	//second order Runge-Kutta method for tracking a path along the gradient direction of a given position, which is also responsible for integrating the gradient field until the stopping criterions are satisfied
	float second_order_Runge_Kutta(vec3f initialPosition,vec3f initialGradient,int integrationStep,vector<nv::vec3f> &gradient,vector<float> &gradient_magnitude,vector<nv::vec3f> &second_derivative,vector<float> &second_derivative_magnitude,int flag);

	//obtain the vector values between voxels during the path tracking procedure
	vec3f trilinearInterpolation(vec3f initialPosition, vector<nv::vec3f> &gradient);

	//obtain the scalar values between voxels during the path tracking procedure
	float trilinearInterpolation(vec3f initialPositon);
};

#endif //class LH_Histograms
