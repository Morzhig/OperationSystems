#include <iostream>
#include <Thread>
#include <windows.h>
#include <Algorithm>
#include <fstream>

using namespace std;

int n, mini, maxi;
int avg = 0;
int* mas;

DWORD WINAPI min_max()
{
    int min = mas[0];
    int max = mas[0];

    for (int i = 0; i < n; i++)
    {
        if (min > mas[i])
        {
            min = mas[i];
            mini = i;
            Sleep(7);
        }

        if (max < mas[i])
        {
            max = mas[i];
            maxi = i;
            Sleep(7);
        }
    }

    cout << "Minimum is: " << min << endl;
    cout << "Maximum is: " << max << endl;

    return 0;
}

DWORD WINAPI average()
{
    for (int i = 0; i < n; i++)
    {
        avg += mas[i];
        Sleep(12);
    }

    avg /= n;

    return 0;
}

int main()
{
    ifstream in("input.txt");

    HANDLE avgThread;
    HANDLE maxiThread;

    DWORD MaxThread;
    DWORD AvgThread;

    cout << "Enter size of array: ";
    cin >> n;

    mas = new int[n];

    cout << "Enter array: ";

    for (int i = 0; i < n; i++)
        cin >> mas[i];

    avgThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)average, 0, 0, &AvgThread);
    maxiThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)min_max, 0, 0, &MaxThread);

    if (avgThread != 0 && maxiThread != 0)
    {
        WaitForSingleObject(avgThread, INFINITE);
        WaitForSingleObject(maxiThread, INFINITE);

        CloseHandle(avgThread);
        CloseHandle(maxiThread);
    }

    cout << "\nArithmetic avergage is: "; // for debug
    cout << avg << endl; // for debug

    cout << "\nFinal array is: ";

    for (int i = 0; i < n; i++)
    {
        if (i == maxi || i == mini)
            mas[i] = avg;
        cout << mas[i] << ' ';
    }

    delete[]mas;

    return 0;
}