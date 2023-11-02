#define WIN32_LEAN_AND_MEAN 

#pragma warning(disable : 4996)

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <WinSock2.h>

#if !defined(_Wp64)
#define DWORD_PTR DWORD
#define LONG_PTR LONG
#define INT_PTR INT
#endif

WORD WINAPI DoWork();

typedef struct _RECORD {
	int AccountNumber;
	int Transaction;
} RECORD, *pRECORD;

// Globals
pRECORD data;
CRITICAL_SECTION* critical_section;

// Fucntion call
WORD WINAPI DoWork(char* fileName);

int _tmain(int argc, LPTSTR argv[])
{

	LARGE_INTEGER FileSize;
	HANDLE STDInput;
	HANDLE STDOutput;
	STDInput = GetStdHandle(STD_INPUT_HANDLE);
	STDOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	GetFileSizeEx(STDInput, &FileSize);
	DWORD BIn;
	DWORD BOut;

	int numOfProcess = argc - 1; // command line arguments
	int sizeoffile = FileSize.QuadPart;
	int numOfAccounts = FileSize.QuadPart / 8; // number of accounts 
	HANDLE* hArr;
	char* fileName;


	// Memory Allocations
	data = malloc(sizeoffile);
	critical_section = malloc(numOfAccounts * sizeof(CRITICAL_SECTION));
	hArr = malloc(numOfProcess * sizeof(HANDLE));	
	
	//read in RECORDS
	for(int i = 0; i < numOfAccounts; i++)
	{
		ReadFile(STDInput, &data[i], sizeof(RECORD), &BIn, NULL);
	}

	// Initialize Critical Sections
	for (int i = 0; i < numOfAccounts; i++)
	{
		InitializeCriticalSection(&critical_section[i]);
	}
	// spawn threads
	for (int i = 1; i <= numOfProcess; i++)
	{
		fileName = malloc(strlen(argv[i]));
		strcpy(fileName, argv[i]);
		hArr[i-1]= (HANDLE)_beginthreadex(NULL, 0, DoWork, fileName, 0, NULL);
	}
	// wait for object
	for (int i = 0; i < numOfProcess; i++)
	{
		WaitForSingleObject(hArr[i], INFINITE);
	}
	// write to STDOUT
	for (int i = 0; i < numOfAccounts; i++)
	{
		WriteFile(STDOutput, &data[i], 8, &BOut, NULL);
	}



	free(hArr);
	free(critical_section);
	free(data);

	return 0;
}

WORD WINAPI DoWork(char* fileName)
{
	LARGE_INTEGER FileSize;
	
	
	
	HANDLE file;
	
	DWORD BIn;
	file = CreateFile(fileName, GENERIC_READ,
		FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	GetFileSizeEx(file, &FileSize);
	
	int numOfAccounts = FileSize.QuadPart / 8;
	
	int accountNum = 0;
	int transaction =0;

	for (int i = 0; i < numOfAccounts; i++)
	{
		
		ReadFile(file, &accountNum, 4, &BIn, NULL);
		ReadFile(file, &transaction, 4, &BIn, NULL);
		
		EnterCriticalSection(&critical_section[accountNum]);

		__try 
		{
			// do transaction
			data[accountNum].Transaction += transaction;
			
		}
		__finally
		{
			LeaveCriticalSection(&critical_section[accountNum]);
		}

	}

	



}