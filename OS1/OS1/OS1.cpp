#define _CRT_SECURE_NO_WARNINGS
#include "Header.h"

using namespace std;

char* CreatorInput(istream& in, char* filename, int number) 
{
	cout << "Enter the name of binary file: ";

	char name[100];
	cin >> name;

	strcpy(filename, name);

	cout << "How many employees do you have?: ";
	cin >> number;

	string s = to_string(number);
	char const* num = s.c_str();

	char* command_line = new char[strlen(filename) + strlen(num) + 2];

	strcpy(command_line, filename);
	strcat(command_line, " ");
	strcat(command_line, num);

	return command_line;
}

char* ReporterInput(istream& in, char* filename, char* report_filename, double salary) 
{
	cout << "Enter the name of report file: ";

	char name[100];
	cin >> name;
	strcpy(report_filename, name);

	cout << "How much money do your employees get? ";
	cin >> salary;

	string s = to_string(salary);
	char const* num = s.c_str();

	char* command_line = new char[strlen(filename) + strlen(report_filename) + strlen(num) + 3];

	strcpy(command_line, filename);
	strcat(command_line, " ");
	strcat(command_line, report_filename);
	strcat(command_line, " ");
	strcat(command_line, num);

	return command_line;
}

LPWSTR CharToLPWSTR(char* str) 
{
	wchar_t wtext[100];
	mbstowcs(wtext, str, strlen(str) + 1);
	LPWSTR ptr = wtext;
	return ptr;
}

LPWSTR GetIpCommandLine(char* app_name, char* command_line) 
{
	char* lpszCommandLine = new char[strlen(app_name) + strlen(command_line) + 2];

	strcpy(lpszCommandLine, app_name);
	strcat(lpszCommandLine, " ");
	strcat(lpszCommandLine, command_line);

	return CharToLPWSTR(lpszCommandLine);
}

int main()
{
	char filename[100];
	int number = 0;

	char* creator_command_line = CreatorInput(cin, filename, number);
	char creator_app_name[15] = "CreatorApp.exe";

	LPWSTR ip_line_creator = GetIpCommandLine(creator_app_name, creator_command_line);

	STARTUPINFO si1;
	PROCESS_INFORMATION pi1;
	ZeroMemory(&si1, sizeof(STARTUPINFO));
	si1.cb = sizeof(STARTUPINFO);

	if (CreateProcess(NULL, ip_line_creator, NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE, NULL, NULL, &si1, &pi1))
		_cputs("The Creator process is created.\n");
	else
	{
		_cputs("The new process was not created.\n");
		_cputs("Check the name of the process.\n");
		_cputs("Press any key to finish.\n");
		_getch();
		return 0;
	}

	WaitForSingleObject(pi1.hProcess, INFINITE);

	CloseHandle(pi1.hThread);
	CloseHandle(pi1.hProcess);

	char report_filename[100];
	int salary = 0;

	char* reporter_command_line = ReporterInput(cin, filename, report_filename, salary);
	char reporter_app_name[16] = "ReporterApp.exe";

	LPWSTR ip_line_reporter = GetIpCommandLine(reporter_app_name, reporter_command_line);

	STARTUPINFO si2;
	PROCESS_INFORMATION pi2;
	ZeroMemory(&si2, sizeof(STARTUPINFO));
	si2.cb = sizeof(STARTUPINFO);

	if (CreateProcess(NULL, ip_line_reporter, NULL, NULL, FALSE,
		CREATE_NEW_CONSOLE, NULL, NULL, &si2, &pi2))
		_cputs("The Reporter process is created.\n");
	else
	{
		_cputs("The new process was not created.\n");
		_cputs("Check the name of the process.\n");
		_cputs("Press any key to finish.\n");
		_getch();
		return 0;
	}

	WaitForSingleObject(pi2.hProcess, INFINITE);

	CloseHandle(pi2.hThread);
	CloseHandle(pi2.hProcess);

	return 0;
}