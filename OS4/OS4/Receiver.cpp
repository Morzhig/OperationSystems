#pragma warning(disable : 4996)
#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <conio.h>
#include <process.h>

CRITICAL_SECTION CriticalS;

int main() 
{
	char filename[100];
	int numOfEnters;

	std::cout << "Input filename:" << std::endl;
	std::cin >> filename;
	std::cin >> numOfEnters;

	int numOfSenders;

	std::cin >> numOfSenders;
	
	InitializeCriticalSection(&CriticalS);
	
	std::ofstream out;
	
	out.open(filename, std::fstream::binary);
	out.close();

	STARTUPINFO* si = new STARTUPINFO[numOfSenders];
	PROCESS_INFORMATION* pi = new PROCESS_INFORMATION[numOfSenders];

	HANDLE* sEvent = new HANDLE[numOfSenders];
	HANDLE* senders = new HANDLE[numOfSenders];

	SECURITY_ATTRIBUTES secureA = 
	{ 
		sizeof(SECURITY_ATTRIBUTES), 0, TRUE 
	};

	std::string eve = "SenderStartEvent";

	const char* Eve = eve.c_str();
	wchar_t* wEve = new wchar_t[4096];

	MultiByteToWideChar(CP_ACP, 0, Eve, -1, wEve, 4096);

	HANDLE evToStart = CreateEvent(&secureA, FALSE, FALSE, wEve);

	for (int i = 0; i < numOfSenders; i++) 
	{
		std::string arg = "Sender.exe";
		arg = arg + " " + filename;

		ZeroMemory(&si[i], sizeof(STARTUPINFO));

		si[i].cb = sizeof(STARTUPINFO);

		ZeroMemory(&pi[i], sizeof(PROCESS_INFORMATION));

		std::string argm = "Event";
		argm = argm + " " + std::to_string(i);

		const char* Argm = argm.c_str();
		wchar_t* wArgm = new wchar_t[4096];
		MultiByteToWideChar(CP_ACP, 0, Eve, -1, wArgm, 4096);

		sEvent[i] = CreateEvent(&secureA, FALSE, FALSE, wArgm);

		arg = arg + " Event " + std::to_string(i);
		
		const char* Arg = arg.c_str();
		wchar_t* wArg = new wchar_t[4096];
		MultiByteToWideChar(CP_ACP, 0, Arg, -1, wArg, 4096);

		if (!CreateProcess(NULL, wArg, NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si[i], &pi[i]))
		{
			std::cout << GetLastError();

			ExitProcess(0);
		}

		senders[i] = pi[i].hProcess;
	}
	WaitForMultipleObjects(numOfSenders, sEvent, TRUE, INFINITE);
	SetEvent(evToStart);

	while (WaitForMultipleObjects(numOfSenders, senders, TRUE, 0) == WAIT_TIMEOUT) 
	{
		std::string message;

		std::cout << "Input your message write 'STOP' to stop: ";
		std::cin >> message;

		if (message == "STOP")
			break;

		std::ifstream in;
		in.open(filename, std::fstream::binary);

		std::string message_;

		int i = 0;

		while (!in.eof())
		{
			in.read((char*)&message_, sizeof(std::string));
			std::cout << message_;
			i++;
		}

		in.close();
		LeaveCriticalSection(&CriticalS);
	}

	for (int i = 0; i < numOfSenders; i++) 
	{
		CloseHandle(pi[i].hThread);
		CloseHandle(pi[i].hProcess);
		CloseHandle(sEvent[i]);
	}

	CloseHandle(evToStart);

	return 0;
}