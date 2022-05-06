#include <stdio.h>
#include "Employee.h"
#include  "Windows.h"

extern CRITICAL_SECTION* Write;
extern CRITICAL_SECTION* Read;
extern CRITICAL_SECTION* readersCounts;
extern HANDLE* FinishSemaphore;
extern employee* data;
extern int empCount;
extern int* curCount;

void ReadInfo(int recordNumber, HANDLE hPipe, DWORD cbWritten, TCHAR* chBuf, DWORD cbRead);
void WriteInfo(int recordNumber, HANDLE hPipe, DWORD cbWritten, TCHAR* chBuf, DWORD cbRead);

DWORD __stdcall handleClient(LPVOID params) {
    DWORD cbWritten, cbRead;
    TCHAR chBuf[BUFSIZE];
    HANDLE hPipe = (HANDLE)params;

    while (TRUE)  {
        int recordNumber = 0;
        ReadFile( hPipe, chBuf, sizeof(chBuf), &cbRead, NULL);

        sscanf(chBuf, "%*c %d", &recordNumber);

        switch (chBuf[0]) {
            case 'r':  ReadInfo(recordNumber, hPipe, cbWritten, chBuf, cbRead);              break;
            case 'w':  WriteInfo(recordNumber, hPipe, cbWritten, chBuf, cbRead);             break;
            case 'q':  ReleaseSemaphore(FinishSemaphore, 1, NULL);   return 0;
            default:   printf("Unknown request %s \n", chBuf);
        }
    }
}

void ReadInfo(int recordNumber, HANDLE hPipe, DWORD cbWritten, TCHAR* chBuf, DWORD cbRead) {
    EnterCriticalSection(&readersCounts[recordNumber - 1]);
    if (++curCount[recordNumber - 1] == 1)
        EnterCriticalSection(&Write[recordNumber - 1]);
    LeaveCriticalSection(&readersCounts[recordNumber - 1]);


    WriteFile(hPipe, data + recordNumber - 1, sizeof(employee), &cbWritten, NULL);
    ReadFile( hPipe, chBuf, sizeof(chBuf), &cbRead,NULL);


    EnterCriticalSection(&readersCounts[recordNumber - 1]);
    if (--curCount[recordNumber - 1] == 0)
        LeaveCriticalSection(&Write[recordNumber - 1]);
    LeaveCriticalSection(&readersCounts[recordNumber - 1]);
}

void WriteInfo(int recordNumber, HANDLE hPipe, DWORD cbWritten, TCHAR* chBuf, DWORD cbRead) {
    EnterCriticalSection(&Write[recordNumber - 1]);
    EnterCriticalSection(&Read[recordNumber - 1]);

    WriteFile(hPipe, data + recordNumber - 1, sizeof(employee), &cbWritten, NULL);
    ReadFile(hPipe, data + recordNumber - 1, sizeof(employee), &cbWritten, NULL);
    ReadFile( hPipe, chBuf, sizeof(chBuf), &cbRead, NULL);

    LeaveCriticalSection(&Read[recordNumber - 1]);
    LeaveCriticalSection(&Write[recordNumber - 1]);
}