#include "stat_reader.h"
namespace input_output
{
    void OutputStop(TransportCatalogue& catalogue, std::string& line, std::istream& input, std::ostream& out)
    {
        size_t name_start = line.find_first_not_of(' ', 4);
        size_t name_end = line.find_last_not_of(' ');
        std::string name = line.substr(name_start, name_end - name_start + 1);

        Stop stop = catalogue.GetStop(name);

        out << "Stop " << name << ": ";
        if (stop.GetName() != name)
        {
            out << "not found" << std::endl;
        }
        else
        {
            std::set<std::string> routes = stop.GetRoutes();
            if (routes.size() == 0)
            {
                out << "no buses" << std::endl;
            }
            else
            {
                out << "buses";
                for (auto it = routes.begin(); it != routes.end(); it++)
                {
                    out << " " << *it;
                }
                out << std::endl;
            }
        }
    }
    void OutputRoute(TransportCatalogue& catalogue, std::string& line, std::istream& input, std::ostream& out)
    {
        size_t name_start = line.find_first_not_of(' ', 4);
        size_t name_end = line.find_last_not_of(' ');
        std::string name = line.substr(name_start, name_end - name_start + 1);
        Route route = catalogue.GetRoute(name);

        out << "Bus " << name << ": ";
        if (route.GetName() != name)
        {
            out << "not found" << std::endl;
        }
        else
        {
            double route_length = catalogue.CalculateRouteLength(name);
            out << route.GetStopsCount() << " stops on route, "
                << route.GetRoutesUniqueStops() << " unique stops, "
                << route_length << " route length, "
                << route_length / catalogue.CalculateRouteLength_RAW(name) << " curvature" << std::endl;
        }
    }

    void GetStats(TransportCatalogue& catalogue, std::istream& input, std::ostream& out)
    {
        std::string line;
        getline(input, line);

        if (line.find("Bus") != std::string::npos)
        {
            // Return route
            OutputRoute(catalogue, line, input, out);
        }
        else
        {
            // Return stop
            OutputStop(catalogue, line, input, out);
        }
    }
}