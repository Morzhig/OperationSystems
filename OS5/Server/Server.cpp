#include <process.h>
#include <windows.h>
#include <iostream>
#include <fstream>
#include <time.h>
#include <conio.h>
#include <algorithm>
#include <string>

struct employee 
{
    int num;
    char name[10];
    double hours;

    void print() 
    {
        std::cout << "ID: " << num << " Name: " << name
            << " Hours: " << hours << std::endl;
    }
};

int empCmp(const void* p1, const void* p2) 
{
    return ((employee*)p1)->num - ((employee*)p2)->num;
}

int empCount;
employee* emps;
HANDLE* handleReadyEvents;
CRITICAL_SECTION empsCS;
bool* empIsModifying;
int employeeSize = sizeof(employee);

const std::string pipeName = "\\\\.\\pipe\\pipe_name";
const int BUFF_LENGTH = 10;
const int MESSAGE_LENGTH = 10;
char buff[BUFF_LENGTH];

int generateCountOfClient() 
{
    srand(time(0));
    return rand();
}

employee* findEmp(const int ID) 
{
    employee key;

    key.num = ID;

    return (employee*)bsearch(&key, emps,
        empCount, employeeSize, empCmp);
}

void sortEmps() 
{
    qsort(emps, empCount, employeeSize, empCmp);
}

void writeData(std::string fileName) 
{
    std::fstream in(fileName.c_str(), std::ios::out | std::ios::binary);
    in.write(reinterpret_cast<const char*>(emps), employeeSize * empCount);

    std::cout << "Data is being written" << std::endl;
    in.close();
}

void readDataSTD() 
{
    emps = new employee[empCount];

    std::cout << "Input ID, name and working hours of each employee: ";
    for (int i = 0; i < empCount; ++i) {
        std::cout << "Emplyee " << i + 1 << ": ";
        std::cin >> emps[i].num >> emps[i].name >> emps[i].hours;
    }
}

void startPocesses(const int count) 
{
    for (int i = 0; i < count; i++) 
    {
        std::string cmdArgs = "..\\..\\Client\\cmake-build-debug\\Client.exe ";
        std::string eventName = "READY_EVENT_";

        itoa(i + 1, buff, BUFF_LENGTH);

        eventName += buff;
        cmdArgs += eventName;

        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        si.cb = sizeof(STARTUPINFO);
        ZeroMemory(&si, sizeof(STARTUPINFO));

        handleReadyEvents[i] = CreateEvent(NULL, TRUE,
            FALSE, eventName.c_str());

        char tempArg[60];
        strcat(tempArg, cmdArgs.c_str());

        if (CreateProcess(NULL, tempArg,
            NULL, NULL,
            FALSE, CREATE_NEW_CONSOLE,
            NULL, NULL,
            &si, &pi) == false) {
            std::cout << "Creation process error." << std::endl;

            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }
}

DWORD WINAPI messaging(LPVOID p) 
{
    employee* errorEmp = new employee;

    errorEmp->num = -1;

    HANDLE hPipe = (HANDLE)p;
    while (true) 
    {
        DWORD readBytes;
        char message[MESSAGE_LENGTH];

        bool isRead = ReadFile(hPipe, message,
            MESSAGE_LENGTH, &readBytes, NULL);

        if (false == isRead) 
        {
            if (ERROR_BROKEN_PIPE == GetLastError()) 
            {
                std::cout << "Client disconnected." << std::endl;
            }
            else {
                std::cerr << "Error in reading a message." << std::endl;
            }
            break;
        }

        if (strlen(message) != 0) 
        {
            char command = message[0];

            message[0] = ' ';

            int id = atoi(message);

            DWORD bytesWritten;
            EnterCriticalSection(&empsCS);
            employee* empToSend = findEmp(id);
            LeaveCriticalSection(&empsCS);

            if (NULL == empToSend)
                empToSend = errorEmp;
            else 
            {
                int ind = empToSend - emps;

                if (empIsModifying[ind])
                    empToSend = errorEmp;
                else 
                {
                    if (command == 'r') 
                        std::cout << "Requested to read ID " << id << ".";
                    else if (command == 'w')
                    {
                        std::cout << "Requested to modify ID " << id << ".";
                        empIsModifying[ind] = true;
                    }
                    else 
                        std::cout << "Unknown request. ";
                }
            }

            bool isSent = WriteFile(hPipe, empToSend,
                employeeSize, &bytesWritten, NULL);

            if (true == isSent) 
                std::cout << "Answer is sent." << std::endl;
            else
                std::cout << "Error in sending answer." << std::endl;

            if (empToSend != errorEmp && 'w' == command) 
            {
                isRead = ReadFile(hPipe, empToSend,
                    employeeSize, &readBytes, NULL);
                if (false == isRead) 
                {
                    std::cerr << "Error in reading a message." << std::endl;
                    break;
                }
                else 
                {
                    std::cout << "Employee record changed." << std::endl;

                    empIsModifying[empToSend - emps] = false;
                    EnterCriticalSection(&empsCS);
                    sortEmps();
                    LeaveCriticalSection(&empsCS);
                }
            }
        }
    }

    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    delete errorEmp;
    return 0;
}

void openPipes(int clientCount) 
{
    HANDLE hPipe;
    HANDLE* hThreads = new HANDLE[clientCount];

    for (int i = 0; i < clientCount; i++) 
    {
        hPipe = CreateNamedPipe(pipeName.c_str(), PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES, 0,
            0, INFINITE, NULL);

        if (!ConnectNamedPipe(hPipe, NULL)) 
        {
            std::cout << "No connected clients.\n";
            break;
        }

        if (INVALID_HANDLE_VALUE == hPipe) 
        {
            std::cerr << "Create named pipe failed.\n";

            getch();

            return;
        }

        hThreads[i] = CreateThread(NULL, 0,
            messaging, (LPVOID)hPipe,
            0, NULL);
    }

    std::cout << "Clients connected to pipe." << std::endl;

    WaitForMultipleObjects(clientCount, hThreads, TRUE, INFINITE);
    std::cout << "All clients are disconnected." << std::endl;

    delete[] hThreads;
}

int main() 
{
    std::string filename;

    std::cout << "Input name of file: ";
    std::cin >> filename;

    std::cout << "Input count of employees: ";
    std::cin >> empCount;

    readDataSTD();
    writeData(filename);
    sortEmps();

    int countOfClient = generateCountOfClient();
    empIsModifying = new bool[empCount];

    for (int i = 0; i < empCount; ++i)
        empIsModifying[i] = false;

    InitializeCriticalSection(&empsCS);
    HANDLE handleStartALL = CreateEvent(NULL, TRUE,
        FALSE, "START_ALL");

    handleReadyEvents = new HANDLE[countOfClient];
    startPocesses(countOfClient);

    WaitForMultipleObjects(countOfClient, handleReadyEvents,
        TRUE, INFINITE);
    std::cout << "All processes are ready.Starting." << std::endl;

    SetEvent(handleStartALL);

    openPipes(countOfClient);

    for (int i = 0; i < empCount; ++i)
        emps[i].print();

    std::cout << "Press any key to exit...\n";
    getch();
    DeleteCriticalSection(&empsCS);

    delete[] empIsModifying;
    delete[] handleReadyEvents;
    delete[] emps;

    return 0;
}