#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>

using namespace std;

struct employee
{
	int num; // идентификационный номер сотрудника
	char name[10]; // имя сотрудника
	double hours; // количество отработанных часов

	employee()
	{
		num = 0;
		hours = 0;
		strcpy(name, "0");
	}

	friend istream& operator>>(istream& in, employee& emp)
	{
		in >> emp.num >> emp.name >> emp.hours;
		return in;
	}

	friend ostream& operator<<(ostream& out, const employee& emp)
	{
		out << emp.num << ' ' << emp.name << ' ' << emp.hours;
		return out;
	}

};

void main(int argc, char* argv[])
{
	char filename[100];
	strcpy(filename, argv[1]);

	int empl_num = atoi(argv[2]);

	ofstream outfile(filename, ios::binary);

	cout << "Enter your employees(number, name, hours): " << endl;
	for (int i = 0; i < empl_num; i++)
	{
		employee temp;

		cin >> temp;

		outfile.write((char*)&temp, sizeof(employee));
	}
}