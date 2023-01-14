#pragma warning(disable: 4996)
#include <iostream>
#include <fstream>
#include <string>
#include <Windows.h>

CRITICAL_SECTION CriticalS;

int main(int argc, char** argv) 
{
	InitializeCriticalSection(&CriticalS);

	std::cout << argv[1] << std::endl;

	wchar_t* wString1 = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, argv[2], -1, wString1, 4096);

	wchar_t* nameEvent = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, "StartSenderEvent", -1, nameEvent, 4096);

	HANDLE ev = OpenEvent(EVENT_MODIFY_STATE, FALSE, wString1);
	HANDLE sEv = OpenEvent(SYNCHRONIZE, FALSE, nameEvent);

	std::ofstream out;

	out.open(argv[1], std::ofstream::binary | std::ios_base::app);

	std::string message;

	SignalObjectAndWait(ev, sEv, INFINITE, FALSE);

	while (true) 
	{
		std::cout << "Input your message write 'STOP' to stop: ";
		std::cin >> message;

		if (message == "STOP")
			break;

		EnterCriticalSection(&CriticalS);
		message += '\n';

		out.write((const char*)&message, sizeof(std::string));

		SetEvent(ev);
		LeaveCriticalSection(&CriticalS);
	}

	CloseHandle(sEv);
	out.close();
	CloseHandle(ev);

	return 0;
}