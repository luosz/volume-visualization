#ifndef  _LH_HISTOGRAMS_CONSTRUCTOR_
#define  _LH_HISTOGRAMS_CONSTRUCTOR_

#include <fstream>
#include <iostream>
#include <vector>
#include <utility>
#include <nvMath.h>
#include "../my_raycasting/VolumeReader.h"
//using namespace std;
//using namespace nv;


//static int COUNT = 0;
class LH_Histograms{
public:
	//epsilon_min for determine whether the gradient magnitude is small enough
	float m_epsilon_min;

	//epsilon_max for determine whether the difference between the gradient and second derivative is big enough
	float m_epsilon_max;

	//integration step for the second order Runge-Kutta method
	int m_integrationStep;

	//pointer to the VolumeReader object
	//VolumeReader *m_volumeReader;
	std::ofstream out2;


	//vector for store the FL,FH information of every voxel in the volume
	std::vector<std::pair<nv::vec3f,std::pair<float,float>>> m_LHInformation;
	
public:
	//constructor and destructor
	LH_Histograms(/*VolumeReader *volumeReader*/)
	{
		m_epsilon_min = 0.5;
		m_epsilon_max = 0.5;
		m_integrationStep = 1;
		//m_volumeReader = volumeReader;
		out2.open("d:/test.txt");
	}

	~LH_Histograms()
	{

	}

	//main procedure for calculate the FL,FH for every voxel
	void constructor(VolumeReader &m_volumeReader,std::vector<nv::vec3f> &gradient, std::vector<float> &gradient_magnitude, std::vector<nv::vec3f> &second_derivative, std::vector<float> &second_derivative_magnitude)
	{
		int i,j,k;
		float FL,FH;
		FL = FH = 0.0;

		nv::vec3f tempPosition;
		for(i = 0; i <= m_volumeReader.getX()-1; i++)
		{
			for(j = 0; j <= m_volumeReader.getY()-1; j++)
			{
				for( k = 0; k <= m_volumeReader.getZ()-1; k++)
				{
					
					int index = m_volumeReader.getIndex(i,j,k);
					tempPosition.x = i;
					tempPosition.y = j;
					tempPosition.z = k;
					if(gradient_magnitude[index] <= m_epsilon_min){
						FL = FH = m_volumeReader.getData(i,j,k); 
					}else{
						FH = second_order_Runge_Kutta(m_volumeReader,tempPosition,gradient[index],m_integrationStep,gradient,gradient_magnitude,second_derivative,second_derivative_magnitude,1);
						FL = second_order_Runge_Kutta(m_volumeReader,tempPosition,gradient[index],m_integrationStep,gradient,gradient_magnitude,second_derivative,second_derivative_magnitude,-1);
					}
					out2<<"Normal at ("<<i<<","<<j<<","<<k<<")\t";
					out2<<"(FL,FH) = "<<FL<<","<<FH<<")"<<std::endl;
					std::pair<float,float> value;
					value = std::make_pair(FL,FH);
					std::pair<nv::vec3f,std::pair<float,float>> positionAndValue;
					positionAndValue = std::make_pair(tempPosition,value);
					m_LHInformation.push_back(positionAndValue);
				}
			}
		}
	}

	//second order Runge-Kutta method for tracking a path along the gradient direction of a given position, which is also responsible for integrating the gradient field until the stopping criterions are satisfied
	float second_order_Runge_Kutta(VolumeReader &m_volumeReader,nv::vec3f initialPosition,nv::vec3f initialGradient,int integrationStep,std::vector<nv::vec3f> &gradient,std::vector<float> &gradient_magnitude,std::vector<nv::vec3f> &second_derivative,std::vector<float> &second_derivative_magnitude,int flag)
	{

		float value = 0.0;
		nv::vec3f direction = flag*initialGradient;
		nv::vec3f k1 = integrationStep*direction;
		nv::vec3f mediumPosition = initialPosition+k1/2;
		if((ceil(mediumPosition.x) >= m_volumeReader.getX()-1) || (ceil(mediumPosition.y) >= m_volumeReader.getY()-1) || (ceil(mediumPosition.z) >= m_volumeReader.getZ()-1) ||
			(floor(mediumPosition.x) <= 0) || (floor(mediumPosition.y) <= 0) || (floor(mediumPosition.z) <= 0)){
				value = trilinearInterpolation(m_volumeReader,initialPosition);
				return value;
		}
		nv::vec3f k2 = trilinearInterpolation(m_volumeReader,mediumPosition,gradient);
		if( k2 == (0.0,0.0,0.0)){
			value = trilinearInterpolation(m_volumeReader,mediumPosition);
			return value;
		}
		nv::vec3f nextPosition = initialPosition+integrationStep*k2;
		if((ceil(nextPosition.x) >= m_volumeReader.getX()-1) || (ceil(nextPosition.y) >= m_volumeReader.getY()-1) || (ceil(nextPosition.z) >= m_volumeReader.getZ()-1) ||
			(floor(nextPosition.x) <= 0) || (floor(nextPosition.y) <= 0) || (floor(nextPosition.z) <= 0)){
				value = trilinearInterpolation(m_volumeReader,nextPosition);
				return value;
		}
		nv::vec3f nextGradient = trilinearInterpolation(m_volumeReader,nextPosition,gradient);
		nv::vec3f nextSecondDerivative = trilinearInterpolation(m_volumeReader,nextPosition,second_derivative);

		//the original paper said that "we track a path by integrating the gradient field in both directions",but the stopping criterion include:reaching a constant area(|gradient[index]|<epsilon),a local extremum or an inflex point(length(nextSecondDerivative) <= m_epsilon_min && length(nextSecondDerivative) >= m_epsilon_max) 
		if(length(nextGradient) <= m_epsilon_min/* && nv::length(nextSecondDerivative) <= m_epsilon_min) || (nv::length(nextSecondDerivative) <= m_epsilon_min && length(nextSecondDerivative) >= m_epsilon_max )*/){
			value = trilinearInterpolation(m_volumeReader,nextPosition);
			return value;
		}else{
			second_order_Runge_Kutta(m_volumeReader,nextPosition,nextGradient,integrationStep,gradient,gradient_magnitude,second_derivative,second_derivative_magnitude,flag);
		}
	}

	//obtain the vector values between voxels during the path tracking procedure
	nv::vec3f trilinearInterpolation(VolumeReader &m_volumeReader,nv::vec3f initialPosition, std::vector<nv::vec3f> &gradient)
	{
		float xd = initialPosition.x - floor(initialPosition.x);
		float yd = initialPosition.y - floor(initialPosition.y);
		float zd = initialPosition.z - floor(initialPosition.z);

		int x_floor = floor(initialPosition.x);
		int y_floor = floor(initialPosition.y);
		int z_floor = floor(initialPosition.z);

		int x_ceil = ceil(initialPosition.x);
		int y_ceil = ceil(initialPosition.y);
		int z_ceil = ceil(initialPosition.z);

		nv::vec3f i1 = gradient[m_volumeReader.getIndex(x_floor,y_floor,z_floor)]*(1-zd)+gradient[m_volumeReader.getIndex(x_floor,y_floor,z_ceil)]*zd;
		nv::vec3f i2 = gradient[m_volumeReader.getIndex(x_floor,y_ceil,z_floor)]*(1-zd)+gradient[m_volumeReader.getIndex(x_floor,y_ceil,z_ceil)]*zd;

		nv::vec3f j1 = gradient[m_volumeReader.getIndex(x_ceil,y_floor,z_ceil)]*(1-zd)+gradient[m_volumeReader.getIndex(x_ceil,y_floor,z_ceil)]*zd;
		nv::vec3f j2 = gradient[m_volumeReader.getIndex(x_ceil,y_ceil,z_floor)]*(1-zd)+gradient[m_volumeReader.getIndex(x_ceil,y_ceil,z_ceil)]*zd;

		nv::vec3f w1 = i1*(1-yd)+i2*yd;
		nv::vec3f w2 = j1*(1-yd)+j2*yd;

		nv::vec3f finalGradient = w1*(1-xd)+w2*xd;
		/*std::cout<<"finalGradient = ("<<finalGradient.x<<","<<finalGradient.y<<","<<finalGradient.z<<")"<<std::endl;*/
		return finalGradient;
	}

	//obtain the scalar values between voxels during the path tracking procedure
	float trilinearInterpolation(VolumeReader &m_volumeReader,nv::vec3f initialPosition)
	{
		float xd = initialPosition.x - floor(initialPosition.x);
		float yd = initialPosition.y - floor(initialPosition.y);
		float zd = initialPosition.z - floor(initialPosition.z);

		int x_floor = floor(initialPosition.x);
		int y_floor = floor(initialPosition.y);
		int z_floor = floor(initialPosition.z);

		int x_ceil = ceil(initialPosition.x);
		int y_ceil = ceil(initialPosition.y);
		int z_ceil = ceil(initialPosition.z);

		
		if( x_floor <= 0 )
			x_floor = 0;
		if(x_ceil <= 0)
			x_ceil = 0;
		if(y_floor <= 0)
			y_floor = 0;
		if(y_ceil <= 0)
			y_ceil = 0;
		if(z_floor <= 0)
			z_floor = 0;
		if(z_ceil <= 0)
			z_ceil = 0;
		if(x_ceil >= 40 )
			x_ceil = 40;
		if (x_floor >= 40)
			x_floor = 40;
		if(y_ceil >= 40)
			y_ceil = 40;
		if(y_floor >= 40)
			y_floor = 40;
		if(z_ceil >= 40)
			z_ceil = 40;
		if(z_floor >= 40)
			z_floor = 40;
		//out<<"get data from ("<<x_floor<<","<<y_floor<<","<<z_floor<<")"<<std::endl;
		//out<<"get data from ("<<x_floor<<","<<y_floor<<","<<z_ceil<<")"<<std::endl;
		float i1 = m_volumeReader.getData(x_floor,y_floor,z_floor)*(1-zd)+m_volumeReader.getData(x_floor,y_floor,z_ceil)*zd;
		//out<<"get data from ("<<x_floor<<","<<y_ceil<<","<<z_floor<<")"<<std::endl;
		//out<<"get data from ("<<x_floor<<","<<y_ceil<<","<<z_ceil<<")"<<std::endl;
		float i2 = m_volumeReader.getData(x_floor,y_ceil,z_floor)*(1-zd)+m_volumeReader.getData(x_floor,y_ceil,z_ceil)*zd;
		/*out<<"get data from ("<<x_ceil<<","<<y_floor<<","<<z_floor<<")"<<std::endl;
		out<<"get data from ("<<x_ceil<<","<<y_floor<<","<<z_ceil<<")"<<std::endl;*/
		float j1 = m_volumeReader.getData(x_ceil,y_floor,z_floor)*(1-zd)+m_volumeReader.getData(x_ceil,y_floor,z_ceil)*zd;
		/*out<<"get data from ("<<x_ceil<<","<<y_ceil<<","<<z_floor<<")"<<std::endl;
		out<<"get data from ("<<x_ceil<<","<<y_ceil<<","<<z_ceil<<")"<<std::endl;*/
		float j2 = m_volumeReader.getData(x_ceil,y_ceil,z_floor)*(1-zd)+m_volumeReader.getData(x_ceil,y_ceil,z_ceil)*zd;


		float w1 = i1*(1-yd)+i2*yd;
		float w2 = j1*(1-yd)+j2*yd;

		float finalScalarValue = w1*(1-xd)+w2*xd;
		return finalScalarValue;
	}	
};

#endif //class LH_Histograms
