#ifndef K_Means_Local_h
#define K_Means_Local_h

namespace clustering
{

#include "kmlocal/KMlocal.h"

	/**	@brief	The k-means local clustering.
	*	
	*	This class is an adapter for the k-means local clustering.
	*	
	*	http://www.cs.umd.edu/~mount/Projects/KMeans/
	*	
	*	http://www.cs.umd.edu/~mount/Projects/KMeans/pami02.pdf
	*	
	*	http://www.cs.umd.edu/~mount/Projects/KMeans/kmlocal-1.7.2.zip
	*/
	class K_Means_Local
	{
	public:
		static void k_means(const unsigned int count, const vector<float> & scalar_value, const vector<float> & gradient_magnitude, const vector<float> & second_derivative_magnitude, const int k, unsigned char *& label_ptr)
		{
			int	dim		= 3;		// dimension
			int	maxPts		= static_cast<int>(count);		// max number of data points
			int	stages		= 100;		// number of stages

			//----------------------------------------------------------------------
			//  Termination conditions
			//	These are explained in the file KMterm.h and KMlocal.h.  Unless
			//	you are into fine tuning, don't worry about changing these.
			//----------------------------------------------------------------------
			KMterm	term(100, 0, 0, 0,		// run for 100 stages
				0.10,			// min consec RDL
				0.10,			// min accum RDL
				3,			// max run stages
				0.50,			// init. prob. of acceptance
				10,			// temp. run length
				0.95);			// temp. reduction factor
			term.setAbsMaxTotStage(stages);		// set number of stages
			KMdata dataPts(dim, maxPts);	// allocate data storage

#if 0
			kmClusGaussPts(dataPts.getPts(), maxPts, dim, k);
#else
			// read the points
			KMpointArray pa = dataPts.getPts();
			for (int i = 0; i < dataPts.getNPts(); i++) {
				pa[i][0] = scalar_value[i];
				pa[i][1] = gradient_magnitude[i];
				pa[i][2] = second_derivative_magnitude[i];
			}
#endif

			dataPts.buildKcTree();			// build filtering structure
			KMfilterCenters ctrs(k, dataPts);		// allocate centers

			// run the k-means algorithm
			KMlocalLloyds km(ctrs, term);
			ctrs = km.execute();			// execute

			// get/print final cluster assignments
			KMctrIdxArray closeCtr = new KMctrIdx[dataPts.getNPts()];
			double* sqDist = new double[dataPts.getNPts()];
			ctrs.getAssignments(closeCtr, sqDist);

			for (int i=0; i<dataPts.getNPts(); i++)
			{
				label_ptr[i] = static_cast<unsigned char>(closeCtr[i]);
			}

			delete [] closeCtr;
			delete [] sqDist;
		}
	};

}

#endif // K_Means_Local_h