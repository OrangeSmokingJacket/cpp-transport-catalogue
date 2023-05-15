#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>

#include "transport_catalogue.h"

namespace input_output
{
	void GetStats(TransportCatalogue& catalogue, std::istream& input, std::ostream& out);
}
