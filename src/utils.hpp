#pragma once
#include <cmath>

inline double haversine(const double lat1, const double lon1, const double lat2, const double lon2) {
    constexpr double R = 6371000.0;
    const double phi1 = lat1 * M_PI / 180.0;
    const double phi2 = lat2 * M_PI / 180.0;
    const double dphi = (lat2 - lat1) * M_PI / 180.0;
    const double dlambda = (lon2 - lon1) * M_PI / 180.0;
    const double a = sin(dphi / 2) * sin(dphi / 2) + cos(phi1) * cos(phi2) * sin(dlambda / 2) * sin(dlambda / 2);

    return 2 * R * std::asin(std::sqrt(a));
}