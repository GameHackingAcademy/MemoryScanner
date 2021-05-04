#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

#define size 0x00000808

void search(const HANDLE process, const int passed_val) {
	FILE* temp_file = NULL;
	fopen_s(&temp_file, "res.txt", "w");

	unsigned char* buffer = (unsigned char*)calloc(1, size);
	DWORD bytes_read = 0;

	for (DWORD i = 0x00000000; i < 0x7FFFFFFF; i += size) {
		ReadProcessMemory(process, (void*)i, buffer, size, &bytes_read);

		for (int j = 0; j < size - 4; j += 4) {
			DWORD val = 0;
			memcpy(&val, &buffer[j], 4);
			if (val == passed_val) {
				fprintf(temp_file, "%x\n", i + j);
			}
		}
	}

	fclose(temp_file);

	free(buffer);
}

void filter(const HANDLE process, const int passed_val) {
	FILE* temp_file = NULL;
	FILE* temp_file_filter = NULL;
	fopen_s(&temp_file, "res.txt", "r");
	fopen_s(&temp_file_filter, "res_fil.txt", "w");

	DWORD address = 0;
	while (fscanf_s(temp_file, "%x\n", &address) != EOF) {
		DWORD val = 0;
		DWORD bytes_read = 0;

		ReadProcessMemory(process, (void*)address, &val, 4, &bytes_read);
		if (val == passed_val) {
			fprintf(temp_file_filter, "%x\n", address);
		}
	}

	fclose(temp_file);
	fclose(temp_file_filter);

	fopen_s(&temp_file, "res.txt", "w");
	fopen_s(&temp_file_filter, "res_fil.txt", "r");
	while (fscanf_s(temp_file_filter, "%x\n", &address) != EOF) {
		fprintf(temp_file, "%x\n", address);
	}

	fclose(temp_file);
	fclose(temp_file_filter);

	remove("res_fil.txt");
}

void write(const HANDLE process, const int passed_val) {
	FILE* temp_file = NULL;
	fopen_s(&temp_file, "res.txt", "r");

	DWORD address = 0;
	while (fscanf_s(temp_file, "%x\n", &address) != EOF) {
		DWORD bytes_written = 0;

		WriteProcessMemory(process, (void*)address, &passed_val, 4, &bytes_written);
	}

	fclose(temp_file);
}

int main(int argc, char** argv) {
	HANDLE process_snapshot = 0;
	PROCESSENTRY32 pe32 = { 0 };

	DWORD exitCode = 0;

	pe32.dwSize = sizeof(PROCESSENTRY32);

	process_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	Process32First(process_snapshot, &pe32);

	do {
		if (wcscmp(pe32.szExeFile, L"wesnoth.exe") == 0) {
			HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, true, pe32.th32ProcessID);

			char* p;
			long value = strtol(argv[2], &p, 10);

			if(strcmp(argv[1], "search") == 0) {
				search(process, value);
			}
			else if(strcmp(argv[1], "filter") == 0) {
				filter(process, value);
			}
			else if (strcmp(argv[1], "write") == 0) {
				write(process, value);
			}

			CloseHandle(process);
			break;
		}
	} while (Process32Next(process_snapshot, &pe32));

	return 0;
}
