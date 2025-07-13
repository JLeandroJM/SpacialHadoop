#pragma once

#include "Index.hpp"
#include <vector>
#include <algorithm>
#include <queue>
#include <cmath>
#include "utils.hpp"

struct Rect {
    double minLat, minLon, maxLat, maxLon;
    Rect() : minLat(0), minLon(0), maxLat(0), maxLon(0) {}
    Rect(double minLat, double minLon, double maxLat, double maxLon)
        : minLat(minLat), minLon(minLon), maxLat(maxLat), maxLon(maxLon) {}
    Rect(const Geoname& g)
        : minLat(g.latitude), minLon(g.longitude), maxLat(g.latitude), maxLon(g.longitude) {}
    void expand(const Rect& r) {
        minLat = std::min(minLat, r.minLat);
        minLon = std::min(minLon, r.minLon);
        maxLat = std::max(maxLat, r.maxLat);
        maxLon = std::max(maxLon, r.maxLon);
    }
    bool contains(const Geoname& g) const {
        return g.latitude >= minLat && g.latitude <= maxLat &&
               g.longitude >= minLon && g.longitude <= maxLon;
    }
    bool intersects(const Rect& r) const {
        return !(r.minLat > maxLat || r.maxLat < minLat ||
                 r.minLon > maxLon || r.maxLon < minLon);
    }
    double area() const {
        return (maxLat - minLat) * (maxLon - minLon);
    }
    static Rect boundingRect(const std::vector<Geoname>& points) {
        if (points.empty()) return Rect();
        double minLat = points[0].latitude, maxLat = points[0].latitude;
        double minLon = points[0].longitude, maxLon = points[0].longitude;
        for (const auto& g : points) {
            minLat = std::min(minLat, g.latitude);
            maxLat = std::max(maxLat, g.latitude);
            minLon = std::min(minLon, g.longitude);
            maxLon = std::max(maxLon, g.longitude);
        }
        return Rect(minLat, minLon, maxLat, maxLon);
    }
};

class RTreeIndex : public Index {
    struct Node {
        bool isLeaf;
        std::vector<Geoname> points; // solo para hojas
        std::vector<Node*> children; // solo para internos
        std::vector<Rect> mbrs; // bounding rectangles de los hijos
        Rect mbr; // bounding rectangle de este nodo
        Node(bool leaf) : isLeaf(leaf) {}
    };
    Node* root;
    int maxDegree;


    // --- STR Build ---
    Node* buildSTR(std::vector<Geoname>& points, int degree, int depth = 0) {
        // std::cout << "construyendo para " << points.size() << " puntos con grado " << degree << "y profundidad " << depth << "\n";
        if (points.empty())
            return nullptr;

        if (points.size() <= degree) {
            const auto leaf = new Node(true);
            leaf->points = points;
            leaf->mbr = Rect::boundingRect(points);
            return leaf;
        }
        // 1. Ordenar por latitud
        std::sort(points.begin(), points.end(), [](const Geoname& a, const Geoname& b) {
            return a.latitude < b.latitude;
        });
        int S = (int)std::ceil((double)points.size() / degree);
        std::vector<std::vector<Geoname>> slices;
        // std::cout << "S: " << S << "\n";
        for (int i = 0; i < points.size(); i += S) {
            int end = std::min(i + S, (int)points.size());
            std::vector<Geoname> slice(points.begin() + i, points.begin() + end);
            // 2. Ordenar cada slice por longitud
            std::sort(slice.begin(), slice.end(), [](const Geoname& a, const Geoname& b) {
                return a.longitude < b.longitude;
            });
            slices.push_back(std::move(slice));
        }
        std::vector<Node*> children;
        std::vector<Rect> mbrs;
        for (auto& slice : slices) {
            for (int i = 0; i < slice.size(); i += degree) {
                int end = std::min(i + degree, (int)slice.size());
                std::vector<Geoname> group(slice.begin() + i, slice.begin() + end);
                auto child = buildSTR(group, degree, depth + 1);
                mbrs.push_back(child->mbr);
                children.push_back(std::move(child));
            }
        }
        const auto node = new Node(false);
        node->children = std::move(children);
        node->mbrs = std::move(mbrs);
        node->mbr = Rect::boundingRect(points);
        return node;
    }

    // --- Range Query ---
    void rangeQueryRec(const Node* node, const Rect& query, std::vector<Geoname>& result) const {
        if (!node) return;
        if (!node->mbr.intersects(query)) return;
        if (node->isLeaf) {
            for (const auto& g : node->points) {
                if (query.contains(g)) result.push_back(g);
            }
        } else {
            for (size_t i = 0; i < node->children.size(); ++i) {
                if (node->mbrs[i].intersects(query))
                    rangeQueryRec(node->children[i], query, result);
            }
        }
    }

    // --- kNN Query ---
    struct QueueEntry {
        const Node* node;
        double minDist;
        bool operator<(const QueueEntry& o) const { return minDist > o.minDist; }
    };
    static double minDistRect(const Rect& r, double lat, double lon) {
        const double clat = std::clamp(lat, r.minLat, r.maxLat);
        const double clon = std::clamp(lon, r.minLon, r.maxLon);
        return haversine(lat, lon, clat, clon);
    }

    template <typename Compare>
    void kNNQuery(
        const Node* node,
        double qLat,
        double qLon,
        int k,
        std::priority_queue<
            std::pair<double, Geoname>,
            std::vector<std::pair<double, Geoname>>,
            Compare> &pq
            ) const {
        if (!node) return;
        if (node->isLeaf) {
            for (const auto& g : node->points) {
                if (qLat == g.latitude && qLon == g.longitude) continue;
                double d = haversine(qLat, qLon, g.latitude, g.longitude);
                if (pq.size() < k)
                    pq.emplace(d, g);
                else if (d < pq.top().first) {
                    pq.pop();
                    pq.emplace(d, g);
                }
            }
        } else {
            // Buscar hijos por orden de distancia mínima
            std::vector<std::pair<double, int>> order;
            for (int i = 0; i < node->children.size(); ++i) {
                double d = minDistRect(node->mbrs[i], qLat, qLon);
                order.emplace_back(d, i);
            }
            std::sort(order.begin(), order.end());
            for (auto [_, i] : order) {
                kNNQuery(node->children[i], qLat, qLon, k, pq);
            }
        }
    }

public:
    explicit RTreeIndex(const int degree = 16) : maxDegree(degree) {}
    void build(const std::vector<Geoname>& points) override {
        if (points.empty()) { root = nullptr; return; }
        std::vector<Geoname> pts = points;
        root = buildSTR(pts, maxDegree);
    }
    std::vector<Geoname> rangeQuery(double minLat, double minLon, double maxLat, double maxLon) override {
        std::vector<Geoname> result;
        if (!root) return result;
        const Rect query(minLat, minLon, maxLat, maxLon);
        rangeQueryRec(root, query, result);
        return result;
    }
    std::vector<Geoname> kNN(const Geoname& q, int k) override {
        using Pair = std::pair<double, Geoname>;

        auto cmp = [](const Pair& a, const Pair& b) {
            return a.first > b.first;
        };

        std::priority_queue<Pair, std::vector<Pair>, decltype(cmp)> pq(cmp);
        kNNQuery(root, q.latitude, q.longitude, k, pq);
        std::vector<Geoname> res;
        while (!pq.empty()) {
            // if (pq.top().second.geonameId != q.geonameId) // evitar el mismo punto
                res.push_back(pq.top().second);
            pq.pop();
        }
        std::reverse(res.begin(), res.end());
        if (res.size() > k) res.resize(k);
        return res;
    }
};