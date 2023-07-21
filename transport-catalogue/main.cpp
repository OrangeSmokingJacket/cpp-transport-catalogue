#include "transport_catalogue.h"
#include "map_renderer.h"
#include "request_handler.h"
#include "json_reader.h"

#include <fstream>
#include <iostream>
#include <string_view>
#include <iomanip>

using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr)
{
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[])
{
    std::cout << std::setprecision(6);

    if (argc != 2)
    {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    if (mode == "make_base"sv)
    {
	    TransportCatalogue catalogue;
	    renderer::MapRenderer renderer;
	    RequestHandler handler(catalogue, renderer);
	    ParseInput(std::cin, handler, mode);
    }
    else if (mode == "process_requests"sv)
    {
        TransportCatalogue catalogue;
        renderer::MapRenderer renderer;
        RequestHandler handler(catalogue, renderer);
        ParseInput(std::cin, handler, mode);
        json::Print(handler.ReturnDocument(), std::cout);
    }
    else
    {
        PrintUsage();
        return 1;
    }
}