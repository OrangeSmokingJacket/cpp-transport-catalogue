#include "transport_catalogue.h"
#include "request_handler.h"

#include <iomanip>


using namespace std;
int main()
{
	std::cout << std::setprecision(6);
	TransportCatalogue catalogue;
	renderer::MapRenderer renderer;
	RequestHandler handler(catalogue, renderer);
	handler.ParseInput(std::cin);
	json::Print(handler.ReturnDocument(), std::cout);
}