#include <iostream>
#include <time.h>
#include <vector>
#include <windows.h>
#include <process.h>

struct threadArgs 
{
    int* arr;
    int n;
    int num;

    HANDLE actions[2];

    threadArgs(int* _arr, int _n, int _num)
    {
        num = _num;
        n = _n;

        arr = new int[n];

        for (int i = 0; i < n; i++)
            arr[i] = _arr[i];

        actions[0] = CreateEvent(NULL, FALSE, FALSE, NULL);
        actions[1] = CreateEvent(NULL, FALSE, FALSE, NULL);
    }
};

HANDLE threadStartEv, currTh;
std::vector<HANDLE> thEvents;
CRITICAL_SECTION CriticalS;
int timeMs = 5;

UINT WINAPI marker(void* p) 
{

    threadArgs* p_ = static_cast<threadArgs*>(p);
    WaitForSingleObject(threadStartEv, INFINITE);

    srand(p_->num);

    std::cout << "Start " << p_->num << " thread.";

    int action = WaitForMultipleObjects(2, p_->actions, FALSE, INFINITE) - WAIT_OBJECT_0;

    int count = 0;

    while (action != 1) 
    {
        EnterCriticalSection(&CriticalS);

        int i = rand();

        if (p_->arr[i] != 0) 
        {
            Sleep(timeMs);

            std::cout << "Thread number " << p_->num << ", marked elements: " << count << ", failed to mark: " << i << std::endl;

            LeaveCriticalSection(&CriticalS);

            Sleep(timeMs);

            SetEvent(thEvents[p_->num + 1]);

            Sleep(timeMs);
        }

        else 
        {
            Sleep(timeMs);

            p_->arr[i] = p_->num;
            count++;

            LeaveCriticalSection(&CriticalS);

            Sleep(timeMs);

            if (action == 1)
                break;
        }
    }

    for (int i = 0; i < p_->n; i++) 
        if (p_->arr[i] == p_->num)
            p_->arr[i] = 0;

    std::cout << "Thread number " << p_->num << " was finished work." << std::endl;

    return NULL;
}

int main() 
{
    int size;

    std::cout << "Input size of your array: ";
    std::cin >> size;

    int* arr = new int[size];

    for (int i = 0; i < size; i++)
        arr[i] = 0;

    int threadsCount;

    std::cout << "Input amount of Threads: ";
    std::cin >> threadsCount;

    threadStartEv = CreateEvent(NULL, TRUE, FALSE, NULL);

    std::vector<HANDLE> threads;
    std::vector<threadArgs*> argsVector;

    threadArgs* currArgs;

    bool* terminated = new bool[threadsCount];

    for (int i = 0; i < threadsCount; i++) 
    {
        currArgs = new threadArgs(arr, size, i);

        currTh = (HANDLE)_beginthreadex(NULL, 0, marker, currArgs, 0, NULL);

        terminated[i] = false;

        thEvents.push_back(CreateEvent(NULL, TRUE, FALSE, NULL));

        argsVector.push_back(currArgs);
        threads.push_back(currTh);
    }

    int terminatedCount = 0;

    InitializeCriticalSection(&CriticalS);
    PulseEvent(threadStartEv);

    while (terminatedCount != threadsCount) 
    {
        WaitForMultipleObjects(threadsCount, &thEvents[0], TRUE, INFINITE);

        EnterCriticalSection(&CriticalS);

        for (int i = 0; i < size; i++)
            std::cout << arr[i] << " ";

        std::cout << std::endl;

        int threadNumToTerminate;

        std::cout << "What thread do you want to terminate: ";
        std::cin >> threadNumToTerminate;

        terminated[threadNumToTerminate - 1] = true;

        SetEvent(argsVector[threadNumToTerminate - 1]->actions[1]);

        WaitForSingleObject(threads[threadNumToTerminate - 1], INFINITE);

        terminatedCount++;

        EnterCriticalSection(&CriticalS);

        for (int i = 0; i < size; i++)
            std::cout << arr[i] << " ";

        std::cout << std::endl;

        for (int i = 0; i < threadsCount; i++)
        {
            if (terminated[i])
                continue;

            ResetEvent(thEvents[i]);

            SetEvent(argsVector[i]->actions[0]);
        }
    }
    return 0;
}