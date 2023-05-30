#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"

#include <iomanip>


using namespace std;
int main()
{
	std::cout << std::setprecision(6);
	TransportCatalogue catalogue;
	renderer::MapRenderer renderer;
	RequestHandler handler(catalogue, renderer);
	ParseInput(std::cin, handler);
	json::Print(handler.ReturnDocument(), std::cout);
}