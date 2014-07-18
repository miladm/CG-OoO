/*******************************************************************************
 * hist.cpp
 * generate histograms
 ******************************************************************************/
#include <math.h>
#include "../lib/list.h"
#include <string>


class hist {
	public:
		hist(long int numBucket, long int lowerBound, long int upperBound);
		hist(long int numBucket_x, long int lowerBound_x, long int upperBound_x, long int numBucket_y, long int lowerBound_y, long int upperBound_y);
		~hist();
		void addElem(long int elem);
		long int findIndx(long int elem);
		void report();
		void report(FILE* file, string histName);
		

	private:
		long int* _histArr;
		long int** _histMat;
		long int _numBucket_x;
		long int _numBucket_y;
		long int _lowerBound_x;
		long int _upperBound_x;
		long int _lowerBound_y;
		long int _upperBound_y;
		long int _step_x;
		long int _step_y;
};
