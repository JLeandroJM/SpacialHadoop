#pragma once

#include "Index.hpp"
#include "utils.hpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include <queue>
#include <iostream>

static constexpr double EARTH_RADIUS = 6'371'000.0; // metros

class GridIndex : public Index {
public:
    /// gx × gy celdas (por defecto 10×10)
    explicit GridIndex(size_t gx = 10, size_t gy = 10);

    void build(const std::vector<Geoname>& records) override;
    std::vector<Geoname> rangeQuery(double minLat, double minLon,
                                    double maxLat, double maxLon) override;
    std::vector<Geoname> kNN(const Geoname& q, int k) override;

private:
    size_t gx_, gy_;
    double minLat_, maxLat_, minLon_, maxLon_;
    double cellHeight_, cellWidth_;

    std::vector<Geoname> allRecords_;
    // cells_[i][j]: registros en fila i (latitud), columna j (longitud)
    std::vector<std::vector<std::vector<Geoname>>> cells_;

    void assignToCells();
    std::pair<size_t, size_t> getCellIndices(double lat, double lon) const;

};


inline GridIndex::GridIndex(size_t gx, size_t gy)
    : gx_(gx), gy_(gy),
      minLat_(0), maxLat_(0), minLon_(0), maxLon_(0),
      cellHeight_(0), cellWidth_(0) {}

inline void GridIndex::build(const std::vector<Geoname>& records) {
    if (records.empty()) return;

    // 1) almacena todos
    allRecords_ = records;

    // 2) calcula bounds globales
    minLat_ = maxLat_ = records[0].latitude;
    minLon_ = maxLon_ = records[0].longitude;
    for (const auto& g : records) {
        minLat_ = std::min(minLat_, g.latitude);
        maxLat_ = std::max(maxLat_, g.latitude);
        minLon_ = std::min(minLon_, g.longitude);
        maxLon_ = std::max(maxLon_, g.longitude);
    }

    // 3) dim celdas
    cellHeight_ = (maxLat_ - minLat_) / static_cast<double>(gy_);
    cellWidth_ = (maxLon_ - minLon_) / static_cast<double>(gx_);

    // 4) prepara estructura cells_
    cells_.assign(gy_, std::vector<std::vector<Geoname>>(gx_));
    // 5) asigna cada registro a su celda
    assignToCells();
}

inline void GridIndex::assignToCells() {
    for (const auto& g : allRecords_) {
        auto [i, j] = getCellIndices(g.latitude, g.longitude);
        cells_[i][j].push_back(g);
    }
}

inline std::pair<size_t, size_t> GridIndex::getCellIndices(const double lat, const double lon) const {
    size_t i =  std::min<double>(
        gy_ - 1,
        std::floor((lat - minLat_) / cellHeight_)
        );

    size_t j = std::min<double>(
        gx_ - 1,
       std::floor((lon - minLon_) / cellWidth_)
       );
    return {i, j};
}

inline std::vector<Geoname> GridIndex::rangeQuery(const double minLat, const double minLon,
                                                  const double maxLat, const double maxLon) {
    if (allRecords_.empty()) {
        std::cout << "[rangeQuery] No hay registros cargados." << std::endl;
        return {};
    }

    auto [i0, j0] = getCellIndices(minLat, minLon);
    auto [i1, j1] = getCellIndices(maxLat, maxLon);

    std::vector<Geoname> result;

    if (i0 > i1) std::swap(i0, i1);
    if (j0 > j1) std::swap(j0, j1);

    for (size_t i = i0; i <= i1; ++i) {
        for (size_t j = j0; j <= j1; ++j) {
            for (auto const& g : cells_[i][j]) {
                if (g.latitude >= minLat && g.latitude <= maxLat &&
                    g.longitude >= minLon && g.longitude <= maxLon) {
                    result.push_back(g);
                }
            }
        }
    }
    return result;
}

inline std::vector<Geoname> GridIndex::kNN(const Geoname& q, int k) {
    using Pair = std::pair<double, Geoname>;
    auto cmp = [](Pair const& a, Pair const& b)
    {
        return a.first < b.first;
    };
    std::priority_queue<Pair, std::vector<Pair>, decltype(cmp)> pq(cmp);

    for (auto const& g : allRecords_) {
        // if (g.geonameId == q.geonameId) continue;

        double d = haversine(q.latitude, q.longitude,
                             g.latitude, g.longitude);

        if (pq.size() < static_cast<size_t>(k)) {
            pq.emplace(d, g);
        }
        else if (d < pq.top().first) {
            pq.pop();
            pq.emplace(d, g);
        }
    }

    std::vector<Geoname> neighbors;
    neighbors.reserve(pq.size());
    while (!pq.empty()) {
        neighbors.push_back(pq.top().second);
        pq.pop();
    }
    std::reverse(neighbors.begin(), neighbors.end());
    return neighbors;
}