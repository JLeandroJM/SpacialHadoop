#pragma once

#include <vector>
#include "Geoname.hpp"

class Index {
public:
    using JoinResult = std::vector<std::pair<Geoname, Geoname>>;
    virtual ~Index() = default;
    virtual void build(const std::vector<Geoname>& points) = 0;
    virtual std::vector<Geoname> rangeQuery(double minLat, double minLon,
                                            double maxLat, double maxLon) = 0;
    virtual std::vector<Geoname> kNN(const Geoname& q, int k) = 0;
};

