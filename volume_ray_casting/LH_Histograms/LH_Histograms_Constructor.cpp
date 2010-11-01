#include "LH_Histograms_Constructor.h"
#include <math.h>

LH_Histograms::LH_Histograms(VolumeReader *volumeReader){
	m_epsilon_min = 0.0;
	m_epsilon_max = 0.5;
	m_integrationStep = 1;
	m_volumeReader = volumeReader;
}

LH_Histograms::~LH_Histograms(){

}

void LH_Histograms::constructor(vector<nv::vec3f> &gradient, vector<float> &gradient_magnitude, vector<nv::vec3f> &second_derivative, vector<float> &second_derivative_magnitude){
	int i,j,k;
	float FL,FH;
	FL = FH = 0.0;
	
	vec3f tempPosition;
	for(i = 0; i < m_volumeReader->length; i++)
	{
		for(j = 0; j < m_volumeReader->width; j++)
		{
			for( k =0; k < m_volumeReader->height; k++)
			{
				int index = i*j*k;
				tempPosition.x = i;
				tempPosition.y = j;
				tempPosition.z = k;
				if(gradient_magnitude[index] <= m_epsilon_min){
					FL = FH = m_volumeReader->getData(i,j,k); 
				}else{
					FH = second_order_Runge_Kutta(tempPosition,gradient[index],m_integrationStep,gradient,gradient_magnitude,second_derivative,second_derivative_magnitude,1);
					FL = second_order_Runge_Kutta(tempPosition,gradient[index],m_integrationStep,gradient,gradient_magnitude,second_derivative,second_derivative_magnitude,-1);
				}

				pair<float,float> value;
				value = make_pair(FL,FH);
				pair<vec3f,pair<float,float>> positionAndValue;
				positionAndValue = make_pair(tempPosition,value);
				m_LHInformation.push_back(positionAndValue);
			}
		}
	}
}

float LH_Histograms::second_order_Runge_Kutta(vec3f initialPosition,vec3f initialGradient,int integrationStep,vector<nv::vec3f> &gradient,vector<float> &gradient_magnitude,vector<nv::vec3f> &second_derivative,vector<float> &second_derivative_magnitude,int flag){
	float value = 0.0;
	vec3f direction = flag*initialGradient;
	vec3f k1 = integrationStep*direction;
	vec3f mediumPosition = initialPosition+k1/2;
	if((ceil(mediumPosition.x) >= m_volumeReader->length-1) || (ceil(mediumPosition.y) >= m_volumeReader->width-1) || (ceil(mediumPosition.z) >= m_volumeReader->height-1) ||
		(floor(mediumPosition.x) <= 0) || (floor(mediumPosition.y) <= 0) || (floor(mediumPosition.z) <= 0)){
		value = trilinearInterpolation(initialPosition);
		return value;
	}
	vec3f k2 = trilinearInterpolation(mediumPosition,gradient);
	vec3f nextPosition = initialPosition+integrationStep*k2;
	if((ceil(nextPosition.x) >= m_volumeReader->length-1) || (ceil(nextPosition.y) >= m_volumeReader->width-1) || (ceil(nextPosition.z) >= m_volumeReader->height-1) ||
		(floor(nextPosition.x) <= 0) || (floor(nextPosition.y) <= 0) || (floor(nextPosition.z) <= 0)){
		value = trilinearInterpolation(nextPosition);
		return value;
	}
	vec3f nextGradient = trilinearInterpolation(nextPosition,gradient);
	vec3f nextSecondDerivative = trilinearInterpolation(nextPosition,second_derivative);

	//the original paper said that "we track a path by integrating the gradient field in both directions",but the stopping criterion include:reaching a constant area(|gradient[index]|<epsilon),a local extremum or an inflex point(length(nextSecondDerivative) <= m_epsilon_min && length(nextSecondDerivative) >= m_epsilon_max) 
	if( (length(nextGradient) <= m_epsilon_min && length(nextSecondDerivative) <= m_epsilon_min) || (length(nextSecondDerivative) <= m_epsilon_min && length(nextSecondDerivative) >= m_epsilon_max )){
		value = trilinearInterpolation(nextPosition);
		return value;
	}else{
		second_order_Runge_Kutta(nextPosition,nextGradient,integrationStep,gradient,gradient_magnitude,second_derivative,second_derivative_magnitude,flag);
	}
}

vec3f LH_Histograms::trilinearInterpolation(vec3f initialPosition, vector<nv::vec3f> &gradient){
	float xd = initialPosition.x - floor(initialPosition.x);
	float yd = initialPosition.y - floor(initialPosition.y);
	float zd = initialPosition.z - floor(initialPosition.z);

	int x_floor = floor(initialPosition.x);
	int y_floor = floor(initialPosition.y);
	int z_floor = floor(initialPosition.z);

	int x_ceil = ceil(initialPosition.x);
	int y_ceil = ceil(initialPosition.y);
	int z_ceil = ceil(initialPosition.z);

	vec3f i1 = gradient[x_floor*y_floor*z_floor]*(1-zd)+gradient[x_floor*y_floor*z_ceil]*zd;
	vec3f i2 = gradient[x_floor*y_ceil*z_floor]*(1-zd)+gradient[x_floor*y_ceil*z_ceil]*zd;

	vec3f j1 = gradient[x_ceil*y_floor*z_ceil]*(1-zd)+gradient[x_ceil*y_floor*z_ceil]*zd;
	vec3f j2 = gradient[x_ceil*y_ceil*z_floor]*(1-zd)+gradient[x_ceil*y_ceil*z_ceil]*zd;

	vec3f w1 = i1*(1-yd)+i2*yd;
	vec3f w2 = j1*(1-yd)+j2*yd;

	vec3f finalGradient = w1*(1-xd)+w2*xd;
	return finalGradient;
}


float LH_Histograms::trilinearInterpolation(vec3f initialPosition){
	float xd = initialPosition.x - floor(initialPosition.x);
	float yd = initialPosition.y - floor(initialPosition.y);
	float zd = initialPosition.z - floor(initialPosition.z);

	int x_floor = floor(initialPosition.x);
	int y_floor = floor(initialPosition.y);
	int z_floor = floor(initialPosition.z);

	int x_ceil = ceil(initialPosition.x);
	int y_ceil = ceil(initialPosition.y);
	int z_ceil = ceil(initialPosition.z);

	float i1 = m_volumeReader->getData(x_floor,y_floor,z_floor)*(1-zd)+m_volumeReader->getData(x_floor,y_floor,z_ceil)*zd;
	float i2 = m_volumeReader->getData(x_floor,y_ceil,z_floor)*(1-zd)+m_volumeReader->getData(x_floor,y_ceil,z_ceil)*zd;

	float j1 = m_volumeReader->getData(x_ceil,y_floor,z_ceil)*(1-zd)+m_volumeReader->getData(x_ceil,y_floor,z_ceil)*zd;
	float j2 = m_volumeReader->getData(x_ceil,y_ceil,z_floor)*(1-zd)+m_volumeReader->getData(x_ceil,y_ceil,z_ceil)*zd;

	float w1 = i1*(1-yd)+i2*yd;
	float w2 = j1*(1-yd)+j2*yd;

	float finalScalarValue = w1*(1-xd)+w2*xd;
	return finalScalarValue;
}