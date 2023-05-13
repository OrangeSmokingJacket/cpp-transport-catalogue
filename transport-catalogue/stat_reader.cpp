#include "stat_reader.h"
namespace input_output
{
    void GetStats(TransportCatalogue& catalogue, std::istream& input)
    {
        std::string line;
        getline(input, line);

        if (line.find("Bus") != std::string::npos)
        {
            // Return route
            size_t name_start = line.find_first_not_of(' ', 4);
            size_t name_end = line.find_last_not_of(' ');
            std::string name = line.substr(name_start, name_end - name_start + 1);
            Route route = catalogue.GetRoute(name);

            std::cout << "Bus " << name << ": ";
            if (route.GetName() != name)
            {
                std::cout << "not found" << std::endl;
            }
            else
            {
                double route_length = catalogue.CalculateRouteLength(name);
                std::cout << route.GetStopsCount() << " stops on route, "
                    << route.GetRoutesUniqueStops() << " unique stops, "
                    << route_length << " route length, "
                    << route_length / catalogue.CalculateRouteLength_RAW(name) << " curvature" << std::endl;
            }
        }
        else
        {
            // Return stop
            size_t name_start = line.find_first_not_of(' ', 4);
            size_t name_end = line.find_last_not_of(' ');
            std::string name = line.substr(name_start, name_end - name_start + 1);

            Stop stop = catalogue.GetStop(name);

            std::cout << "Stop " << name << ": ";
            if (stop.GetName() != name)
            {
                std::cout << "not found" << std::endl;
            }
            else
            {
                std::set<std::string> routes = stop.GetRoutes();
                if (routes.size() == 0)
                {
                    std::cout << "no buses" << std::endl;
                }
                else
                {
                    std::cout << "buses";
                    for (auto it = routes.begin(); it != routes.end(); it++)
                    {
                        std::cout << " " << *it;
                    }
                    std::cout << std::endl;
                }
            }

        }
    }
}