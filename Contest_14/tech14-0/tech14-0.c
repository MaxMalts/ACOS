#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>



struct ThreadArgs {
	int64_t N;
	int64_t k;
	double* arr;
	int64_t curInd;
};

void* Thread(void* voidArgs) {
	assert(voidArgs != NULL);

	struct ThreadArgs* args = (struct ThreadArgs*)voidArgs;
	assert(args->arr != NULL);
	assert(args->curInd >= 0 && args->curInd < args->k);

	int64_t leftInd = args->curInd - 1;
	int64_t rightInd = args->curInd + 1;

	if (-1 == leftInd) {
		leftInd = args->k - 1;
	}
	if (args->k == rightInd) {
		rightInd = 0;
	}

	for (int64_t i = 0; i < args->N; ++i) {
		args->arr[args->curInd] += 1;
		args->arr[leftInd] += 0.99;
		args->arr[rightInd] += 1.01;
	}

	return NULL;
}


int main(const int argc, const char* argv[]) {
	if (argc != 3) {
		return -1;
	}

	int64_t N = strtol(argv[1], NULL, 10);
	int64_t k = strtol(argv[2], NULL, 10);

	double arr[k];

	struct ThreadArgs threadsArgs[k];
	pthread_t threads[k];
	for (int64_t i = 0; i < k; ++i) {
		threadsArgs[i].N = N;
		threadsArgs[i].k = k;
		threadsArgs[i].arr = arr;
		threadsArgs[i].curInd = i;

		if (pthread_create(&threads[i], NULL, Thread, &threadsArgs[i]) != 0) {
			return -1;
		}
	}

	for (int64_t i = 0; i < k; ++i) {
		if (pthread_join(threads[i], NULL) != 0) {
			return -1;
		}
	}

	for (int64_t i = 0; i < k; ++i) {
		printf("%.10g\n", arr[i]);
	}

	return 0;
}