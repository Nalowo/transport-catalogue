#pragma once

namespace geo {

struct Coordinates {

    Coordinates(double lat_in, double lng_in): lat(lat_in), lng(lng_in) {}

    double lat; //долгота
    double lng; //широта

    bool operator==(const Coordinates other) const {
        return lat == other.lat && lng == other.lng;
    }

    bool operator!=(const Coordinates other) const {
        return !(*this == other);
    }
};

double ComputeDistance(Coordinates from, Coordinates to);

}  // namespace geo