#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <ctype.h>
//#include <stdio.h>


//#define DEBUG

#ifdef DEBUG
#include <assert.h>
#else
#define assert(arg) 
#endif


typedef int32_t value_t;



value_t FileValueByInd(int file, int index) {
	assert(file >= 0);
	assert(index >= 0);

	value_t val = 0;
	lseek(file, index * sizeof(value_t), SEEK_SET);
	read(file, &val, sizeof(value_t));

	return val;
}


int Compare(value_t val1, value_t val2) {
	return val1 < val2;
}


void FileSwapByInd(int file, int ind1, int ind2) {
	assert(ind1 >= 0);
	assert(ind2 >= 0);

	value_t val1 = 0, val2 = 0;

	lseek(file, ind1 * sizeof(value_t), SEEK_SET);
	read(file, &val1, sizeof(value_t));

	lseek(file, ind2 * sizeof(value_t), SEEK_SET);
	read(file, &val2, sizeof(value_t));

	lseek(file, ind2 * sizeof(value_t), SEEK_SET);
	write(file, &val1, sizeof(value_t));

	lseek(file, ind1 * sizeof(value_t), SEEK_SET);
	write(file, &val2, sizeof(value_t));
}


void FileQuickSort_rec(int file, int low, int high) {
	assert(file >= 0);

	if (low >= high)
		return;

	int pivotInd = (high + low) / 2;
	value_t pivot = 0;
	lseek(file, pivotInd * sizeof(value_t), SEEK_SET);
	read(file, &pivot, sizeof(value_t));

	int left = low, right = high;
	while(left <= right) {
		while(Compare(FileValueByInd(file, left), pivot))
			++left;
		while(Compare(pivot, FileValueByInd(file, right)))
			--right;

		if(left <= right) {
			FileSwapByInd(file, left, right);
			++left;
			--right;
		}
		
	}
	FileQuickSort_rec(file, low, right);
	FileQuickSort_rec(file, left, high);

	return;
}


void FileQuickSort(int file) {
	assert(file >= 0);

	int32_t fileSize = (int32_t)lseek(file, 0, SEEK_END);

	assert(fileSize % sizeof(value_t) == 0);
	int nValues = fileSize / sizeof(value_t);

	FileQuickSort_rec(file, 0, nValues - 1);
}


int main(int argc, const char* argv[]) {

	if (argc != 2) 
		return -1;

	int file = open(argv[1], O_RDWR);
	if(-1 == file) {
		return -1;
	}

	FileQuickSort(file);

	close(file);
	return 0;
}