#include "input_reader.h"

namespace input_output
{
    void GetInput(TransportCatalogue& catalogue, std::istream& input)
    {
        std::string line;
        getline(input, line);

        if (line == "")
            return;

        if (line.find("Stop") == 0)
        {
            // This will be stop
            size_t colon = line.find(':');
            size_t name_start = line.find_first_not_of(' ', 4);
            size_t name_end = line.find_last_not_of(' ', colon - 1);

            size_t comma = line.find(',', name_end);
            size_t longitude_start = line.find_first_not_of(' ', colon + 1);
            size_t longitude_end = line.find_last_not_of(' ', comma - 1);
            size_t latitude_start = line.find_first_not_of(' ', comma + 1);
            size_t latitude_end = line.find_first_of(", ", latitude_start) - 1;

            std::vector<std::pair<std::string, double>> distances;

            size_t prev_splitter = latitude_end;
            size_t splitter = prev_splitter;
            if (line.find(',', prev_splitter + 1) != std::string::npos)
            {
                while (splitter != std::string::npos)
                {
                    splitter = line.find(',', prev_splitter + 2);
                    size_t dst_start = line.find_first_not_of(' ', prev_splitter + 2);
                    size_t dst_end = line.find_first_of(' ', dst_start) - 2;

                    size_t stop_start = line.find_first_not_of(' ', line.find("to", dst_end) + 2);
                    size_t stop_end = line.find_last_not_of(' ', splitter - 1);

                    prev_splitter = splitter;

                    distances.push_back({ line.substr(stop_start, stop_end - stop_start + 1), std::stod(line.substr(dst_start, dst_end - dst_start + 1)) });
                }
            }

            catalogue.AddStop
            (
                move(line.substr(name_start, name_end - name_start + 1)),
                { std::stod(line.substr(longitude_start, longitude_end - longitude_start + 1)),
                std::stod(line.substr(latitude_start, latitude_end - latitude_start + 1)) },
                distances
            );
        }
        else
        {
            // Assuming input is correct, whis will be route
            size_t colon = line.find(':');
            size_t name_start = line.find_first_not_of(' ', 4);
            size_t name_end = line.find_last_not_of(' ', colon - 1);

            std::vector<std::string> stops;
            size_t prev_splitter = colon;
            size_t splitter = colon;
            char divisor = line[line.find_first_of(">-", prev_splitter + 1)];
            while (splitter != std::string::npos)
            {
                splitter = line.find(divisor, prev_splitter + 1);
                size_t stop_start = line.find_first_not_of(' ', prev_splitter + 1);
                size_t stop_end = line.find_last_not_of(' ', splitter - 1);
                prev_splitter = splitter;

                stops.push_back(move(line.substr(stop_start, stop_end - stop_start + 1)));
            }
            if (divisor == '>')
                catalogue.AddRoute(move(line.substr(name_start, name_end - name_start + 1)), stops, true);
            else
                catalogue.AddRoute(move(line.substr(name_start, name_end - name_start + 1)), stops, false);
        }
    }
}