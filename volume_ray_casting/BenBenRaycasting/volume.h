#ifndef _VOLUME_
#define _VOLUME_

#include <cstdio>
#include <cstdlib>
#include <cmath>

/**	@brief	rgb triple to store r, g, b color component
*	
*	
*	
*/
typedef struct
{
	unsigned char r;
	unsigned char g;
	unsigned char b;
}Color;

/**	@brief	grid is used to store coordinate of a given voxel in the volume space
*	
*	
*	
*/
typedef struct
{
	unsigned int x;
	unsigned int y;
	unsigned int z;
}grid;

/**	@brief	structure LH to store L and H in LH histogram 
*	
*	
*	
*/
typedef struct
{
	unsigned short FL;
	unsigned short FH;
}LH;

/**	@brief	class volume to store dataset and other data
*	metrics that are computed, such as gradient magnitude,
*	second derivative, average value, deviation, etc. All the           
*	calculations of the volume data are implemented in class 
*   volume.
*
*/ 
class Volume
{
protected:
	unsigned int length;               ///length
	unsigned int width;                ///width
	unsigned int height;               ///height
	unsigned int count;                  ///number of voxels
	char format[50];                     ///data's format
	void * data;                            ///pointer to store volume data
	unsigned int range;                  ///data value's range [0, range]
	unsigned int * histogram;            ///pointer to store histogram
	unsigned short dataTypeSize;         ///number of bytes of the data type
	unsigned int * gradient;             ///gradient magnitude
	unsigned int * df2;                     /// second derivative                    
	unsigned int * df3;                      /// third derivative
	Color * color;                            /// color in rgb space
	float * opacity;                         /// opacity
	float * local_entropy;                /// local entropy
	float * average;                        /// statistical property average
	float * variation;                       /// statistical property variation
	float local_entropy_max;         /// store maximum local entropy
	float max_variation;               /// store maximum variation
	float ex, var, cv;       
	unsigned char * group;             
	bool * tag;
	unsigned int threshold;
	unsigned int max_data, min_data;               /// maximum and minimum data value
	unsigned int max_grad, max_df2, max_df3, min_grad, min_df2, min_df3, max_frequency; /// maximum and minimum gradient , second derivative and third derivative
	float max_ep, min_ep;                               /// maximum and minimum elasity
	float * ep, little_epsilon;
	float * acc_distribution;
	int intensity_gradient_histogram[12][12];
	float spatial_distribution[12][12];
	LH * LH_Histogram;                                /// LH histogram
public:
	Volume()
	{
		length = width = height = count = range = dataTypeSize = max_data = min_data = threshold = 0;
		max_frequency = 0;
		max_grad =  min_grad = 0;
		min_df2 = min_df3 = max_ep = min_ep = 0;
		data = NULL;
		local_entropy_max = 0;
		local_entropy = NULL;
		histogram = NULL;
		color = NULL;
		opacity = NULL;
		group = NULL;
		tag = NULL;
		gradient = NULL;
		df2 = NULL;
		df3 = NULL;
		average = NULL;
		variation = NULL;
		little_epsilon = 10;
	}
	virtual ~Volume()
	{
		if(data)
			free(data);
		if(histogram)
			free(histogram);
		if(gradient)
			free(gradient);
		if(opacity)
			free(opacity);
		if(group)
			free(group);
		if(tag)	
			free(tag);
		if(df2)
			free(df2);
		if(df3)
			free(df3);
		if(average)
			free(average);
		if(variation)
			free(variation);
	}

	/**	@brief	read data discription file, .dat file format 
	*	
	*/
	virtual bool readVolFile(char * s);

	/**	@brief	read dataset from file named s
	*	
	*/
	bool readData(char * s);

	/**	@brief	traverse all the voxels of a dataset
	*	
	*/
	void traverse();

	/**	@brief	calculate histogram of a dataset
	*	
	*/
	void calHistogram();

	/**	@brief	compute second derivative of the volume 
	*	
	*/
	void calDf2();

	/**	@brief	calculate third derivative of the dataset 
	*	
	*/
	void calDf3();

	/**	@brief calculate gradient magnitude 	of the dataset, using central difference
	*	
	*/
	void calGradient();

	/**	@brief	calculate gradient magnitude of the dataset, using central difference too
	*	
	*/
	void calGrad();

	/**	@brief	calculate gradient magnitude of the dataset, using exponent function to approximate
	*	           
	*/
	void calGrad_ex();                   //use f(x) = a * exp(bx)

	/**	@brief	calculate elasticity of all the voxels to approximate gradient magnitude
	*	
	*/
	void calEp();

	/**	@brief	calculate LH histogram
	*	
	*/
	void calLH();

	/**	@brief	calculate local entropy of all the voxels
	*	
	*/
	void calLocalEntropy();

	/**	@brief	calculate local statistical property - average value 
	*	
	*/
	void calAverage();

	/**	@brief	calculate local statistical property - variation
	*	
	*/
	void calVariation();

	/**	@brief	calculate intensity-gradient magnitude scatter plot
	*	
	*/
	void Intensity_gradient_histogram();

	/**	@brief	return data value at position (x, y, z)
	*	
	*/
	virtual unsigned int getData(unsigned int x, unsigned int y, unsigned int z);

	/**	@brief return gradient magnitude at position (x, y, z)	
	*	
	*/
	unsigned int getGrad(unsigned int x, unsigned int y, unsigned int z);

	/**	@brief	return second derivative at position (x, y, z)
	*	
	*/
	unsigned int getDf2(unsigned int x, unsigned int y, unsigned int z);

	/**	@brief	return third derivative at position (x, y, z)
	*	
	*/
	unsigned int getDf3(unsigned int x, unsigned int y, unsigned int z);

	/**	@brief	return elasticity at position (x, y, z)
	*	
	*/
	float   getEp(unsigned int x, unsigned int y, unsigned int z);

	/**	@brief	return average value at position (x, y, z)
	*	
	*/
	float getAverage(unsigned int x, unsigned int y, unsigned int z);

	/**	@brief	return variation at position (x, y, z)
	*	
	*/
	float getVariation(unsigned int x, unsigned int y, unsigned int z);

	/**	@brief	return maximum variation of the dataset
	*	
	*/
	float getMaxVariation(void);

	/**	@brief	return array's index of the dataset at position (x, y, z)
	*	
	*/
	unsigned int getIndex(unsigned int x, unsigned int y, unsigned int z);

	/**	@brief	return number of voxels of dimension x
	*	
	*/
	unsigned int getX(void);

	/**	@brief	return number of voxels of dimension y
	*	
	*/
	unsigned int getY(void);

	/**	@brief	return number of voxels of dimension z
	*	
	*/
	unsigned int getZ(void);

	/**	@brief	return number of voxels of the dataset
	*	
	*/
	unsigned int getCount(void);

	/**	@brief	return maximum data value 
	*	
	*/
	unsigned int getMaxData(void);

	/**	@brief	return minimum data value
	*	
	*/
	unsigned int getMinData(void);

	/**	@brief	return maximum gradient magnitude 
	*	
	*/
	unsigned int getMaxGrad(void);

	/**	@brief	return minimum gradient magnitude 
	*	
	*/
	unsigned int getMinGrad(void);

	/**	@brief	return maximum second derivative 
	*	
	*/
	unsigned int getMaxDf2(void);

	/**	@brief	return minimum second derivative 
	*	
	*/
	unsigned int getMinDf2(void);

	/**	@brief	return maximum third derivative 
	*	
	*/
	unsigned int getMaxDf3(void);

	/**	@brief	return minimum third derivative 
	*	
	*/
	unsigned int getMinDf3(void);

	/**	@brief	return data value's range 
	*	
	*/
	unsigned int getRange(void);

	/**	@brief	return local entropy at (x, y, z)
	*	
	*/
	float getLocalEntropy(unsigned int x, unsigned int y, unsigned int z);

	/**	@brief	return maximum local entropy
	*	
	*/
	float getLocalEntropyMax();

	/**	@brief	return number of voxels of data value = intensity
	*	
	*/
	unsigned int getHistogram(unsigned int intensity);

	/**	@brief	return data's spatial distribution at (i, j)
	*	
	*/
	float getSpatialDistribution(int i, int j);

	/**	@brief	return getIntensity_gradient_histogram at (i, j)
	*	
	*/
	unsigned int getIntensity_gradient_histogram(int i, int j);

	/**	@brief	return maximum frequency
	*	
	*/
	unsigned int getMaxFrequency();

	/**	@brief	return dataset's format
	*	
	*/
	char * getFormat(void);

	/**	@brief	return pointer points to the data stored in memory
	*	
	*/
	void * getDataAddr(void);

	/**	@brief	judge if all the voxels are traversed
	*	
	*/
	bool allTraverse(bool * tag);

	/**	@brief	set tag of position at (x, y, z)
	*	
	*/
	void setTag(unsigned int x, unsigned int y, unsigned int z);

	/**	@brief	return tag at (x, y, z)
	*	
	*/
	bool getTag(unsigned int x, unsigned int y, unsigned int z);

	/**	@brief	set group at (x, y, z)
	*	
	*/
	void setGroup(unsigned int x, unsigned int y, unsigned int z, unsigned int id);

	/**	@brief	return group identifier at (x, y, z)
	*	
	*/
	unsigned int getGroup(unsigned int x, unsigned int y, unsigned int z);

	/**	@brief	visit grid a
	*	
	*/
	bool visit(grid a, grid seed);

	/**	@brief	visit seed's neighbors 
	*	
	*/
	void visitNeighbor(grid seed);

	/**	@brief	cluster all the voxels 
	*	
	*/
	void cluster();

	/**	@brief	report all the data metrics 
	*	
	*/
	void getInfo();

	/**	@brief	calculate statistical properties 
	*	
	*/
	void statistics();

	/**	@brief	return maximum elasticity 
	*	
	*/
	float getMaxEp();

	/**	@brief	return minimum elasticity
	*	
	*/
	float getMinEp();

	/**	@brief	test if voxels comply to normal distribution
	*	
	*/
	void NormalDistributionTest();

	/**	@brief	filter voxels to shape edges 
	*	
	*/
	void filter();

	/**	@brief	calculate average value and deviation value
	*	
	*/
	void average_deviation();
};

#endif