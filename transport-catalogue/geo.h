#pragma once

#include <string>
#include <cmath>


namespace polar_coordinates
{
    struct Coordinates
    {
        double lat;
        double lng;
        Coordinates() = default;
        Coordinates(double latitude, double longitude)
        {
            lat = latitude;
            lng = longitude;
        }
        bool operator==(const Coordinates& other) const
        {
            return lat == other.lat && lng == other.lng;
        }
        bool operator!=(const Coordinates& other) const
        {
            return !(*this == other);
        }
    };

    inline double ComputeDistance(Coordinates from, Coordinates to)
    {
        using namespace std;

        if (from == to)
            return 0;

        static const double dr = 3.1415926535 / 180.0;
        return acos(sin(from.lat * dr) * sin(to.lat * dr) + cos(from.lat * dr) * cos(to.lat * dr) * cos(abs(from.lng - to.lng) * dr)) * 6371000;
    }
}