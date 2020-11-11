
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
//#include <stdio.h>


enum Error {
	noInput_error = 1,
	noOutput_error = 2,
	other_error = 3
};


enum FilePermission {
	rwAll = 0666
};


int StartTask(int fin, int fout1, int fout2) {

	char curChar = 0;
	ssize_t charsRead = read(fin, &curChar, 1);
	while(charsRead > 0) {

		ssize_t charsWritten = 0;
		if (isdigit(curChar))
			charsWritten = write(fout1, &curChar, 1);
		else
			charsWritten = write(fout2, &curChar, 1);

		if (charsWritten != 1)
			return other_error;

		charsRead = read(fin, &curChar, 1);

	}

	if (-1 == charsRead)
		return other_error;

	return 0;

}


int main(const int argv, const char* argc[]) {
	const int NFiles = 3;

	if (argv != NFiles + 1){
		return other_error;
	}

	int fin = open(argc[1], O_RDONLY);
	if (-1 == fin)
		if (ENOENT == errno)
			return noInput_error;
		else
			return other_error;

	int fout1 = open(argc[2], O_WRONLY | O_CREAT, rwAll);
	if (-1 == fout1) {
		close(fin);
		return noOutput_error;
	}

	int fout2 = open(argc[3], O_WRONLY | O_CREAT, rwAll);
	if (-1 == fout2) {
		close(fin);
		close(fout1);
		return noOutput_error;
	}

	int ans = StartTask(fin, fout1, fout2);

	close(fin);
	close(fout1);
	close(fout2);
	return ans;
}