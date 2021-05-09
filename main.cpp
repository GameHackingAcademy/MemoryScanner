/*
A memory scanner for Wesnoth that allows a user to search, filter, and edit memory inside the process. This code can be adapted to 
any target and is intended to show how tools like CheatEngine work.

The scanner has three main operations:
- search
- filter
- write

The search operation will scan all memory from 0x00000000 to 0x7FFFFFFF and use ReadProcessMemory to determine if the address holds
a certain value. Due to ReadProcessMemory failing if a process doesn't have access to an address, the memory is scanned in blocks.
Any values that match are saved to res.txt

The filter operation iterates over all addresses in res.txt to determine if they match the provided value. If so, they are saved
to res_fil.txt. At the end, res_fil.txt is copied over to res.txt.

The write operation uses WriteProcessMemory to write a passed value to all address in res.txt

CreateToolhelp32Snapshot is used to find the Wesnoth process and OpenProcess is used to retrieve a handle. 

The full explanation for how this code works is available at: https://gamehacking.academy/lesson/35
*/
#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>

#define size 0x00000808

// The search function scans all memory from 0x00000000 to 0x7FFFFFFF for the passed value
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

// The filter function takes a list of addresses in res.txt and checks to see
// if they match the provided value. If so, they are written to res_fil.txt
// After the initial pass, filter writes all the addresses in res_fil.txt to res.txt
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

// The write function writes a value to every address in res.txt
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

// The main function is retrieving a process handle to Wesnoth, parsing the program's arguments and passing 
// execution to the proper operation
int main(int argc, char** argv) {
	HANDLE process_snapshot = 0;
	PROCESSENTRY32 pe32 = { 0 };

	pe32.dwSize = sizeof(PROCESSENTRY32);

	
	// The snapshot code is a reduced version of the example code provided by Microsoft at 
	// https://docs.microsoft.com/en-us/windows/win32/toolhelp/taking-a-snapshot-and-viewing-processes
	process_snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	Process32First(process_snapshot, &pe32);

	do {
		// Only retrieve a process handle for Wesnoth
		if (wcscmp(pe32.szExeFile, L"wesnoth.exe") == 0) {
			// Retrieve a process handle so that we can read and write the game's memory
			HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, true, pe32.th32ProcessID);

			// Convert the second parameter to a DWORD-like value
			char* p;
			long value = strtol(argv[2], &p, 10);

			// Depending on the first argument, pass execution to the search, filter, or write operations
			if(strcmp(argv[1], "search") == 0) {
				search(process, value);
			}
			else if(strcmp(argv[1], "filter") == 0) {
				filter(process, value);
			}
			else if (strcmp(argv[1], "write") == 0) {
				write(process, value);
			}

			// Close the process handle
			CloseHandle(process);
			break;
		}
	} while (Process32Next(process_snapshot, &pe32));

	return 0;
}
