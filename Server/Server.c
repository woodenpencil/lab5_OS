#include <stdio.h>
#include "Windows.h"

#define BUFSIZE 20

typedef struct employee {
    int    num;
    char   name[10];
    double hours;
} employee;

CRITICAL_SECTION *Write, *Read, *readersCounts;
HANDLE *FinishSemaphore;
employee *data;
int empCount;
int *curCount;
extern DWORD __stdcall handleClient(LPVOID params);

int main() {
    LPTSTR pipeName = TEXT("\\\\.\\pipe\\accessPipe");
    char *fName = (char *) calloc(30, sizeof(char));
    int clientCount = 0;

    printf("Enter file name\n");
    scanf("%s", fName);
    HANDLE file = CreateFile(fName, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    printf("Enter employees count\n");
    scanf("%d", &empCount);
    employee *emps = (employee *) calloc(empCount, sizeof(employee));

    for (int i = 0; i < empCount; i++) {
        char   name  [10];
        int    num   = 0;
        double hours = 0;

        printf("Enter emp %d data\n", i + 1);
        printf("num: ");        scanf("%d", &num);
        printf("name: ");       scanf("%s", name);
        printf("hours: ");      scanf("%lf", &hours);
        puts("");

        strcpy(emps[i].name, name);
        emps[i].num = num;
        emps[i].hours = hours;

        WriteFile(file, &emps[i], sizeof(employee), NULL, NULL);
    }

    DWORD dwFileSize = GetFileSize(file, NULL);
    HANDLE map = CreateFileMapping(file, NULL, PAGE_READWRITE, 0, 0, NULL);
    data = (employee*)MapViewOfFile(map,FILE_MAP_READ|FILE_MAP_WRITE,0,0,dwFileSize);

    Read          = (CRITICAL_SECTION*) calloc(empCount, sizeof(CRITICAL_SECTION));
    Write         = (CRITICAL_SECTION*) calloc(empCount, sizeof(CRITICAL_SECTION));
    curCount      = (int*)              calloc(empCount, sizeof(int));
    readersCounts = (CRITICAL_SECTION*) calloc(empCount, sizeof(CRITICAL_SECTION));

    for (int i = 0; i < empCount; ++i) {
        CRITICAL_SECTION sectionW;        InitializeCriticalSection(&sectionW);
        CRITICAL_SECTION sectionR;        InitializeCriticalSection(&sectionR);
        CRITICAL_SECTION sectionReaders;  InitializeCriticalSection(&sectionReaders);

        curCount[i]      = 0;
        Write[i]         = sectionW;
        Read[i]          = sectionR;
        readersCounts[i] = sectionReaders;
    }

    for (size_t i = 0; i < empCount; i++)
        printf("employee %llu info:\n  number: %d, name: %s, hours: %lf\n", i + 1, emps[i].num, emps[i].name, emps[i].hours);

    puts("Enter client count");
    scanf("%d", &clientCount);

    FinishSemaphore = CreateSemaphore(NULL, 0, clientCount - 1, "FinishSemaphore");

    for (int i = 0; i < clientCount; i++) {
        char senderFullName[500];
        sprintf(senderFullName, "Client.exe");

        STARTUPINFO cif;
        ZeroMemory(&cif, sizeof(STARTUPINFO));
        cif.cb = (sizeof(STARTUPINFO));
        PROCESS_INFORMATION pi;
        CreateProcess(NULL, senderFullName, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &cif, &pi);

        HANDLE hPipe = CreateNamedPipe(pipeName, PIPE_ACCESS_DUPLEX, PIPE_TYPE_MESSAGE | PIPE_WAIT,
                                       3, sizeof(DWORD), sizeof(DWORD), 100, NULL);

        if (hPipe == INVALID_HANDLE_VALUE) {
            printf("Failure!!!\n");
            return GetLastError();
        }

        ConnectNamedPipe(hPipe, NULL);
        CreateThread(NULL, 0, handleClient, (LPVOID)hPipe, 0, NULL);
    }

    for (int i = 0; i < clientCount; ++i)
        WaitForSingleObject(FinishSemaphore, INFINITE);

    SetFilePointer(file, 0, NULL, FILE_BEGIN);

    int i = 1;
    employee emp;
    DWORD numOfBytesRead;

    printf("\nFile:\n");
    while (ReadFile(file, &emp, sizeof(emp), &numOfBytesRead, NULL)) {
        if (numOfBytesRead == 0) break;
        printf("employee %d info:\n\tnumber: %d, name: %s, hours : %lf\n", i++, emp.num, emp.name, emp.hours);
    }
    return 0;
}
