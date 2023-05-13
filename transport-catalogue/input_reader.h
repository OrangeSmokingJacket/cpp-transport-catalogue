#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

#include "transport_catalogue.h"

namespace input_output
{
	void GetInput(TransportCatalogue& catalogue, std::istream& input);
}