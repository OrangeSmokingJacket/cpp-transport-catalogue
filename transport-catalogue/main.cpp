#include <iostream>
#include <string>

#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace std;
using namespace input_output;

void GetAllInputs(TransportCatalogue& catalogue)
{
	string line;
	getline(cin, line);
	int n = stoi(line);
	for (int i = 0; i < n; i++)
	{
		GetInput(catalogue, cin);
	}
}
void GetAllOutputs(TransportCatalogue& catalogue)
{
	string line;
	getline(cin, line);
	int n = stoi(line);
	for (int i = 0; i < n; i++)
	{
		GetStats(catalogue, cin, cout);
	}
}

int main()
{
	std::cout << std::setprecision(6);
	TransportCatalogue catalogue;

	GetAllInputs(catalogue);
	GetAllOutputs(catalogue);
}