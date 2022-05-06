#include <stdio.h>
#include "Employee.h"
#include  "Windows.h"

int main() {
    LPTSTR pipename = TEXT("\\\\.\\pipe\\accessPipe");
    HANDLE* hPipe = INVALID_HANDLE_VALUE;

    while (hPipe == INVALID_HANDLE_VALUE)
        hPipe = CreateFile(pipename, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

    printf("Client started\n");

    while (TRUE) {
        char* msg = (char*)calloc(30, sizeof(char));
        printf("Enter command.\n r - read \n w - write\n q - quit\n");
        scanf("%s", msg);

        puts("Enter record number");

        DWORD cbWritten;
        int recordNumber = 0;

        if (msg[0] != 'q') {
            scanf("%d", &recordNumber);

            sprintf(msg, "%c %d", msg[0], recordNumber);

            if (WriteFile(hPipe, msg, sizeof(msg), &cbWritten, NULL) == FALSE) {
                printf("fail\n");
                return GetLastError();
            }
        }
        switch (msg[0]) {
            case 'r':  {
                employee emp;
                ReadFile(hPipe, &emp, sizeof(emp), &cbWritten, NULL);
                printf("Employee %d info:\n num = %d\n name = %s\n hours = %f\n", recordNumber, emp.num, emp.name, emp.hours);
                break;
            }
            case 'w':   {
                employee emp;
                ReadFile(hPipe, &emp, sizeof(emp), &cbWritten, NULL);
                printf("Employee %d info:\n num = %d\n name = %s\n hours = %f\n", recordNumber, emp.num, emp.name, emp.hours);

                int    num   = 0;
                char   name  [10];
                double hours = 0;

                puts("Enter employee data");
                printf("num: ");    scanf("%d", &num);
                printf("name: ");   scanf("%s", name);
                printf("hours: ");  scanf("%lf", &hours);

                strcpy(emp.name, name);
                emp.num = num;
                emp.hours = hours;

                puts("Enter any key to send modified record");
                getchar(); getchar();

                WriteFile(hPipe, &emp, sizeof(emp), &cbWritten, NULL);
                break;
            }
            case 'q':
                WriteFile(hPipe, "quit", sizeof("quit"), &cbWritten, NULL);
                return 0;
            default:
                printf("Unknown command %c \n", msg[0]);
                break;
        }

        printf("Enter any key to free access to record\n");
        getchar(); if (msg[0] == 'r') getchar();

        WriteFile(hPipe, "Free record", sizeof("Free record"), &cbWritten, NULL);
    }
}
