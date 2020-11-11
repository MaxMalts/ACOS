#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <windows.h>



enum {
	maxIntStrLen = 15
};


#pragma pack(1)
struct Item {
	int value;
	uint32_t next_pointer;
};
#pragma pack()


int StartTask(HANDLE fin, HANDLE fout) {

	struct Item curItem = { 0, 0 };
	do {

		if (SetFilePointer(fin, curItem.next_pointer, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
			return -1;

		DWORD bytesRead = 0;
		if (ReadFile(fin, &curItem, sizeof(curItem), &bytesRead, NULL) == 0 || bytesRead != sizeof(curItem))
			return -1;

		char curValStr[maxIntStrLen + 1] = "";
		snprintf(curValStr, sizeof(curValStr), "%d ", curItem.value);

		if (WriteFile(fout, curValStr, strlen(curValStr), NULL, NULL) == 0)
			return -1;

	} while (curItem.next_pointer != 0);

	return 0;
}


int main(const int argc, const char* argv[]) {
	if (argc != 2)
		return -1;

	HANDLE fin = CreateFileA(argv[1], GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == fin)
		return -1;

	HANDLE fout = GetStdHandle(STD_OUTPUT_HANDLE);
	if (INVALID_HANDLE_VALUE == fout) {
		CloseHandle(fin);
		return -1;
	}

	int err = StartTask(fin, fout);

	CloseHandle(fin);
	CloseHandle(fout);
	return err;
}