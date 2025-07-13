#pragma once

#include <string>

struct Geoname {
    long geonameId;
    std::string name;
    double latitude = 0.0;
    double longitude = 0.0;
    Geoname() : latitude(0.0f), longitude(0.0f) {}
    Geoname(float x, float y) : latitude(x), longitude(y) {}
    
    float distanceTo(const Geoname &other) const {
        float dx = latitude - other.latitude;
        float dy = longitude - other.longitude;
        return std::sqrt(dx * dx + dy * dy);
    }
};
