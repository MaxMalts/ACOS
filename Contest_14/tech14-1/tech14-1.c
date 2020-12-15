#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>


pthread_mutex_t readLock = PTHREAD_MUTEX_INITIALIZER;

void* SumNumbers(void* arg) {
	int64_t* res = (int64_t*)calloc(1, sizeof(int64_t));

	int64_t curNum = 0;
	int nRead = 0;
	do {
		*res += curNum;

		pthread_mutex_lock(&readLock);
		nRead = scanf("%ld", &curNum);
		pthread_mutex_unlock(&readLock);
	} while (nRead > 0);

	return (void*)res;
}


int main(const int argc, const char* argv[]) {
	if (argc != 2) {
		return -1;
	}

	int64_t N = strtol(argv[1], NULL, 10);

	pthread_t threads[N];
	int partSums[N];
	for (int i = 0; i < N; ++i) {
		if (pthread_create(&threads[i], NULL, SumNumbers, &partSums[i]) != 0) {
			return -1;
		}
	}

	int64_t ans = 0;
	for (int64_t i = 0; i < N; ++i) {
		int64_t* curSum = NULL;
		if (pthread_join(threads[i], (void*)&curSum) != 0) {
			return -1;
		}

		ans += *curSum;
		free(curSum);
	}
	pthread_mutex_destroy(&readLock);

	printf("%ld\n", ans);

	return 0;
}