#include "hist.h"

//1D histogram
hist::hist(long int numBucket, long int lowerBound, long int upperBound) {
	_numBucket_x = numBucket;
	_numBucket_y = 1;
	_lowerBound_x = lowerBound;
	_upperBound_x = upperBound;
	_lowerBound_y = -1;
	_upperBound_y = -1;
	_histMat = new long int* [_numBucket_y];
	for (int i = 0; i < _numBucket_y; i++) {
		_histMat[i] = new long int [_numBucket_x];
		//Initialize
		for (int j = 0; j < _numBucket_x; j++) {
			_histMat[i][j] = 0;
		}
	}

	_step_x = floor((double)(_upperBound_x-_lowerBound_x)/(double)_numBucket_x);
	_step_y = -1;
}

//2D histogram
hist::hist(long int numBucket_x, long int lowerBound_x, long int upperBound_x, long int numBucket_y, long int lowerBound_y, long int upperBound_y) {
	_numBucket_x = numBucket_x;
	_numBucket_y = numBucket_y;
	_lowerBound_x = lowerBound_x;
	_upperBound_x = upperBound_x;
	_lowerBound_y = lowerBound_y;
	_upperBound_y = upperBound_y;
	_histMat = new long int* [_numBucket_y];
	for (int i = 0; i < _numBucket_y; i++) {
		_histMat[i] = new long int [_numBucket_x];
		//Initialize
		for (int j = 0; j < _numBucket_x; j++) {
			_histMat[i][j] = 0;
		}
	}
	_step_x = floor((double)(_upperBound_x-_lowerBound_x)/(double)_numBucket_x);
	_step_y = floor((double)(_upperBound_y-_lowerBound_y)/(double)_numBucket_y);
}

hist::~hist() {
	delete [] _histMat;
}

//TODO redo this function
void hist::addElem(long int elem) {
	long int indx = findIndx(elem);
	if (!(indx < _numBucket_x && indx > -1)) printf("index = %ld,%ld,%ld\n",indx,_step_x,elem);
	Assert(indx < _numBucket_x && indx > -1);
	_histMat[0][indx]++;
}

long int hist::findIndx(long int elem) {
	return floor((long double)elem/(long double)_step_x);
}

void hist::report(FILE* file, string histName) {
	long int total = 0;
	fprintf(file, "----------------%s\n", histName.c_str());
	printf("----------------%s\n", histName.c_str());
	for (int j = 0; j < _numBucket_y; j++) {
		for (int i = 0; i < _numBucket_x; i++) {
			fprintf(file, "%ld, ", _histMat[j][i]);
			printf("%ld, ", _histMat[j][i]);
			total += _histMat[j][i];
		}
		printf("\t");
	}
	printf("\n");
	fprintf(file, "\n");
}

void hist::report() {
	long int total = 0;
	for (int j = 0; j < _numBucket_y; j++) {
		for (int i = 0; i < _numBucket_x; i++) {
			printf("%ld, ", _histMat[j][i]);
			total += _histMat[j][i];
		}
		printf("\t");
	}
	printf("\n");
}
