#include <vector>
#include <memory>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <queue>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <list>
#include <mutex>
#include <filesystem>
#include <cstring>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;
namespace fs = std::filesystem;

// === GEOMETRIC PRIMITIVES ===
// Añadir después de la definición de Point2D
struct Point2D { 
    double x, y; 
    Point2D() : x(0), y(0) {}
    Point2D(double x, double y) : x(x), y(y) {}
    
    // Añadir operador < para permitir el uso en std::pair con std::less
    bool operator<(const Point2D& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
    
    void serialize(std::ostream& out) const {
        out.write(reinterpret_cast<const char*>(&x), sizeof(double));
        out.write(reinterpret_cast<const char*>(&y), sizeof(double));
    }
    
    void deserialize(std::istream& in) {
        in.read(reinterpret_cast<char*>(&x), sizeof(double));
        in.read(reinterpret_cast<char*>(&y), sizeof(double));
    }
};

// Añadir después de la definición de Point3D
struct Point3D { 
    double x, y, z; 
    Point3D() : x(0), y(0), z(0) {}
    Point3D(double x, double y, double z) : x(x), y(y), z(z) {}
    
    // Añadir operador < para permitir el uso en std::pair con std::less
    bool operator<(const Point3D& other) const {
        if (x != other.x) return x < other.x;
        if (y != other.y) return y < other.y;
        return z < other.z;
    }
    
    void serialize(std::ostream& out) const {
        out.write(reinterpret_cast<const char*>(&x), sizeof(double));
        out.write(reinterpret_cast<const char*>(&y), sizeof(double));
        out.write(reinterpret_cast<const char*>(&z), sizeof(double));
    }
    
    void deserialize(std::istream& in) {
        in.read(reinterpret_cast<char*>(&x), sizeof(double));
        in.read(reinterpret_cast<char*>(&y), sizeof(double));
        in.read(reinterpret_cast<char*>(&z), sizeof(double));
    }
};

struct Rectangle {
    double x1, y1, x2, y2;
    Rectangle() : x1(0), y1(0), x2(0), y2(0) {}
    Rectangle(double x1, double y1, double x2, double y2) : x1(x1), y1(y1), x2(x2), y2(y2) {}
    
    bool contains(const Point2D &p) const { 
        return p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2; 
    }
    
    bool intersects(const Rectangle &o) const { 
        return !(o.x1 > x2 || o.x2 < x1 || o.y1 > y2 || o.y2 < y1); 
    }
    
    double area() const { return (x2 - x1) * (y2 - y1); }
    
    Rectangle enlarge(const Point2D &p) const {
        return Rectangle(std::min(x1, p.x), std::min(y1, p.y), 
                        std::max(x2, p.x), std::max(y2, p.y));
    }
    
    Rectangle enlarge(const Rectangle &r) const {
        return Rectangle(std::min(x1, r.x1), std::min(y1, r.y1), 
                        std::max(x2, r.x2), std::max(y2, r.y2));
    }
    
    void serialize(std::ostream& out) const {
        out.write(reinterpret_cast<const char*>(&x1), sizeof(double));
        out.write(reinterpret_cast<const char*>(&y1), sizeof(double));
        out.write(reinterpret_cast<const char*>(&x2), sizeof(double));
        out.write(reinterpret_cast<const char*>(&y2), sizeof(double));
    }
    
    void deserialize(std::istream& in) {
        in.read(reinterpret_cast<char*>(&x1), sizeof(double));
        in.read(reinterpret_cast<char*>(&y1), sizeof(double));
        in.read(reinterpret_cast<char*>(&x2), sizeof(double));
        in.read(reinterpret_cast<char*>(&y2), sizeof(double));
    }
};

struct Polygon {
    std::vector<Point2D> vertices;
    
    Polygon() {}
    Polygon(const std::vector<Point2D>& points) : vertices(points) {}
    
    Rectangle getBoundingBox() const {
        if (vertices.empty()) return Rectangle();
        
        double minX = vertices[0].x, maxX = vertices[0].x;
        double minY = vertices[0].y, maxY = vertices[0].y;
        
        for (const auto& p : vertices) {
            minX = std::min(minX, p.x);
            maxX = std::max(maxX, p.x);
            minY = std::min(minY, p.y);
            maxY = std::max(maxY, p.y);
        }
        
        return Rectangle(minX, minY, maxX, maxY);
    }
    
    bool contains(const Point2D& p) const {
        if (vertices.size() < 3) return false;
        
        bool inside = false;
        for (size_t i = 0, j = vertices.size() - 1; i < vertices.size(); j = i++) {
            if (((vertices[i].y > p.y) != (vertices[j].y > p.y)) &&
                (p.x < (vertices[j].x - vertices[i].x) * (p.y - vertices[i].y) / 
                       (vertices[j].y - vertices[i].y) + vertices[i].x)) {
                inside = !inside;
            }
        }
        return inside;
    }
    
    void serialize(std::ostream& out) const {
        size_t size = vertices.size();
        out.write(reinterpret_cast<const char*>(&size), sizeof(size_t));
        for (const auto& v : vertices) {
            v.serialize(out);
        }
    }
    
    void deserialize(std::istream& in) {
        size_t size;
        in.read(reinterpret_cast<char*>(&size), sizeof(size_t));
        vertices.resize(size);
        for (auto& v : vertices) {
            v.deserialize(in);
        }
    }
};

// === UTILITY FUNCTIONS ===
double distance2D(const Point2D &a, const Point2D &b) {
    double dx = a.x - b.x, dy = a.y - b.y;
    return std::sqrt(dx*dx + dy*dy);
}

double distance3D(const Point3D &a, const Point3D &b) {
    double dx = a.x - b.x, dy = a.y - b.y, dz = a.z - b.z;
    return std::sqrt(dx*dx + dy*dy + dz*dz);
}

// === DISK STORAGE MANAGER ===
class DiskStorageManager {
private:
    std::string baseDir;
    std::string dataFile;
    std::string indexFile;
    std::fstream dataStream;
    std::fstream indexStream;
    std::unordered_map<size_t, size_t> pageOffsets; // pageId -> file offset
    size_t nextOffset = 0;
    std::mutex mutex;
    
public:
    DiskStorageManager(const std::string& dir) : baseDir(dir) {
        fs::create_directories(baseDir);
        dataFile = baseDir + "/data.bin";
        indexFile = baseDir + "/index.bin";
        
        // Open or create files
        dataStream.open(dataFile, std::ios::binary | std::ios::in | std::ios::out | std::ios::app);
        if (!dataStream.is_open()) {
            dataStream.open(dataFile, std::ios::binary | std::ios::out);
            dataStream.close();
            dataStream.open(dataFile, std::ios::binary | std::ios::in | std::ios::out);
        }
        
        loadIndex();
    }
    
    ~DiskStorageManager() {
        saveIndex();
        if (dataStream.is_open()) dataStream.close();
    }
    
    void savePage(size_t pageId, const std::string& data) {
        std::lock_guard<std::mutex> lock(mutex);
        
        size_t dataSize = data.size();
        size_t offset;
        
        auto it = pageOffsets.find(pageId);
        if (it != pageOffsets.end()) {
            offset = it->second;
            dataStream.seekp(offset);
        } else {
            dataStream.seekp(0, std::ios::end);
            offset = dataStream.tellp();
            pageOffsets[pageId] = offset;
        }
        
        dataStream.write(reinterpret_cast<const char*>(&dataSize), sizeof(size_t));
        dataStream.write(data.c_str(), dataSize);
        dataStream.flush();
    }
    
    std::string loadPage(size_t pageId) {
        std::lock_guard<std::mutex> lock(mutex);
        
        auto it = pageOffsets.find(pageId);
        if (it == pageOffsets.end()) {
            return "";
        }
        
        dataStream.seekg(it->second);
        size_t dataSize;
        dataStream.read(reinterpret_cast<char*>(&dataSize), sizeof(size_t));
        
        std::string data(dataSize, '\0');
        dataStream.read(&data[0], dataSize);
        
        return data;
    }
    
    void deletePage(size_t pageId) {
        std::lock_guard<std::mutex> lock(mutex);
        pageOffsets.erase(pageId);
    }
    
private:
    void saveIndex() {
        std::ofstream out(indexFile, std::ios::binary);
        size_t count = pageOffsets.size();
        out.write(reinterpret_cast<const char*>(&count), sizeof(size_t));
        
        for (const auto& [pageId, offset] : pageOffsets) {
            out.write(reinterpret_cast<const char*>(&pageId), sizeof(size_t));
            out.write(reinterpret_cast<const char*>(&offset), sizeof(size_t));
        }
    }
    
    void loadIndex() {
        std::ifstream in(indexFile, std::ios::binary);
        if (!in.is_open()) return;
        
        size_t count;
        in.read(reinterpret_cast<char*>(&count), sizeof(size_t));
        
        for (size_t i = 0; i < count; ++i) {
            size_t pageId, offset;
            in.read(reinterpret_cast<char*>(&pageId), sizeof(size_t));
            in.read(reinterpret_cast<char*>(&offset), sizeof(size_t));
            pageOffsets[pageId] = offset;
            nextOffset = std::max(nextOffset, offset);
        }
    }
};

// === LRU CACHE ===
template<typename K, typename V>
class LRUCache {
private:
    size_t capacity;
    std::list<std::pair<K, V>> itemList;
    std::unordered_map<K, typename std::list<std::pair<K, V>>::iterator> itemMap;
    mutable std::mutex mutex;
    
public:
    LRUCache(size_t cap) : capacity(cap) {}
    
    // Delete copy operations
    LRUCache(const LRUCache&) = delete;
    LRUCache& operator=(const LRUCache&) = delete;
    
    // Move operations
    LRUCache(LRUCache&&) = default;
    LRUCache& operator=(LRUCache&&) = default;
    
    bool get(const K& key, V& value) {
        std::lock_guard<std::mutex> lock(mutex);
        
        auto it = itemMap.find(key);
        if (it == itemMap.end()) {
            return false;
        }
        
        // Move to front
        itemList.splice(itemList.begin(), itemList, it->second);
        value = it->second->second;
        return true;
    }
    
    void put(const K& key, const V& value) {
        std::lock_guard<std::mutex> lock(mutex);
        
        auto it = itemMap.find(key);
        if (it != itemMap.end()) {
            // Update existing
            it->second->second = value;
            itemList.splice(itemList.begin(), itemList, it->second);
            return;
        }
        
        // Insert new
        if (itemList.size() >= capacity) {
            // Remove LRU
            auto& lru = itemList.back();
            itemMap.erase(lru.first);
            itemList.pop_back();
        }
        
        itemList.emplace_front(key, value);
        itemMap[key] = itemList.begin();
    }
    
    void remove(const K& key) {
        std::lock_guard<std::mutex> lock(mutex);
        
        auto it = itemMap.find(key);
        if (it != itemMap.end()) {
            itemList.erase(it->second);
            itemMap.erase(it);
        }
    }
    
    void clear() {
        std::lock_guard<std::mutex> lock(mutex);
        itemList.clear();
        itemMap.clear();
    }
    
    void setCapacity(size_t newCapacity) {
        std::lock_guard<std::mutex> lock(mutex);
        capacity = newCapacity;
        while (itemList.size() > capacity) {
            auto& lru = itemList.back();
            itemMap.erase(lru.first);
            itemList.pop_back();
        }
    }
};

// === STORAGE STRUCTURES ===
struct DataPage {
    std::vector<Point2D> points2D;
    std::vector<Point3D> points3D;
    std::vector<Polygon> polygons;
    Rectangle mbr;
    size_t pageId;
    bool dirty = false;
    
    void updateMBR() {
        if (points2D.empty() && points3D.empty() && polygons.empty()) {
            mbr = Rectangle();
            return;
        }
        
        double minX = std::numeric_limits<double>::max();
        double maxX = std::numeric_limits<double>::lowest();
        double minY = std::numeric_limits<double>::max();
        double maxY = std::numeric_limits<double>::lowest();
        
        for (const auto& p : points2D) {
            minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
            minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
        }
        
        for (const auto& p : points3D) {
            minX = std::min(minX, p.x); maxX = std::max(maxX, p.x);
            minY = std::min(minY, p.y); maxY = std::max(maxY, p.y);
        }
        
        for (const auto& poly : polygons) {
            Rectangle polyMBR = poly.getBoundingBox();
            minX = std::min(minX, polyMBR.x1); maxX = std::max(maxX, polyMBR.x2);
            minY = std::min(minY, polyMBR.y1); maxY = std::max(maxY, polyMBR.y2);
        }
        
        mbr = Rectangle(minX, minY, maxX, maxY);
    }
    
    size_t getMemorySize() const {
        return points2D.size() * sizeof(Point2D) + 
               points3D.size() * sizeof(Point3D) + 
               polygons.size() * sizeof(Polygon);
    }
    
    std::string serialize() const {
        std::ostringstream oss;
        
        // Write counts
        size_t count2D = points2D.size();
        size_t count3D = points3D.size();
        size_t countPoly = polygons.size();
        
        oss.write(reinterpret_cast<const char*>(&count2D), sizeof(size_t));
        oss.write(reinterpret_cast<const char*>(&count3D), sizeof(size_t));
                oss.write(reinterpret_cast<const char*>(&countPoly), sizeof(size_t));
        
        // Write MBR
        mbr.serialize(oss);
        
        // Write 2D points
        for (const auto& p : points2D) {
            p.serialize(oss);
        }
        
        // Write 3D points
        for (const auto& p : points3D) {
            p.serialize(oss);
        }
        
        // Write polygons
        for (const auto& poly : polygons) {
            poly.serialize(oss);
        }
        return oss.str();
    }

    void deserialize(const std::string& data) {
        std::istringstream iss(data);
        
        // Read counts
        size_t count2D, count3D, countPoly;
        iss.read(reinterpret_cast<char*>(&count2D), sizeof(size_t));
        iss.read(reinterpret_cast<char*>(&count3D), sizeof(size_t));
        iss.read(reinterpret_cast<char*>(&countPoly), sizeof(size_t));
        
        // Read MBR
        mbr.deserialize(iss);
        
        // Read 2D points
        points2D.resize(count2D);
        for (auto& p : points2D) {
            p.deserialize(iss);
        }
        
        // Read 3D points
        points3D.resize(count3D);
        for (auto& p : points3D) {
            p.deserialize(iss);
        }
        
        // Read polygons
        polygons.resize(countPoly);
        for (auto& poly : polygons) {
            poly.deserialize(iss);
        }
    }
};

// === RTREE NODE ===
struct RTreeNode {
    Rectangle mbr;
    std::vector<std::shared_ptr<RTreeNode>> children;
    size_t dataPageId = std::numeric_limits<size_t>::max();
    bool isLeaf = false;
    size_t nodeId;
    bool dirty = false;
    
    static const size_t MAX_ENTRIES = 50;
    static const size_t MIN_ENTRIES = 20;
    
    void updateMBR() {
        if (children.empty()) {
            mbr = Rectangle();
            return;
        }
        
        mbr = children[0]->mbr;
        for (size_t i = 1; i < children.size(); ++i) {
            mbr = mbr.enlarge(children[i]->mbr);
        }
        dirty = true;
    }
    
    std::string serialize() const {
        std::ostringstream oss;
        
        // Write node info
        oss.write(reinterpret_cast<const char*>(&nodeId), sizeof(size_t));
        oss.write(reinterpret_cast<const char*>(&isLeaf), sizeof(bool));
        oss.write(reinterpret_cast<const char*>(&dataPageId), sizeof(size_t));
        
        // Write MBR
        mbr.serialize(oss);
        
        // Write children count and IDs
        size_t childCount = children.size();
        oss.write(reinterpret_cast<const char*>(&childCount), sizeof(size_t));
        
        for (const auto& child : children) {
            size_t childId = child->nodeId;
            oss.write(reinterpret_cast<const char*>(&childId), sizeof(size_t));
            child->mbr.serialize(oss);
        }
        
        return oss.str();
    }
    
    void deserialize(const std::string& data, const std::unordered_map<size_t, std::shared_ptr<RTreeNode>>& nodeMap) {
        std::istringstream iss(data);
        
        // Read node info
        iss.read(reinterpret_cast<char*>(&nodeId), sizeof(size_t));
        iss.read(reinterpret_cast<char*>(&isLeaf), sizeof(bool));
        iss.read(reinterpret_cast<char*>(&dataPageId), sizeof(size_t));
        
        // Read MBR
        mbr.deserialize(iss);
        
        // Read children
        size_t childCount;
        iss.read(reinterpret_cast<char*>(&childCount), sizeof(size_t));
        
        children.clear();
        for (size_t i = 0; i < childCount; ++i) {
            size_t childId;
            Rectangle childMBR;
            iss.read(reinterpret_cast<char*>(&childId), sizeof(size_t));
            childMBR.deserialize(iss);
            
            // Child will be loaded on demand
            auto childNode = std::make_shared<RTreeNode>();
            childNode->nodeId = childId;
            childNode->mbr = childMBR;
            children.push_back(childNode);
        }
    }
};

// === BASE INDEX CLASS ===
class SpatialIndex {
public:
    virtual void insert2D(const Point2D& p) = 0;
    virtual void insert3D(const Point3D& p) = 0;
    virtual void insertPolygon(const Polygon& poly) = 0;
    virtual std::vector<Point2D> rangeQuery2D(const Rectangle& window) = 0;
    virtual std::vector<Point3D> rangeQuery3D(const Rectangle& window) = 0;
    virtual std::vector<Polygon> rangeQueryPolygon(const Rectangle& window) = 0;
    virtual std::vector<Point2D> knnQuery2D(const Point2D& p, int k) = 0;
    virtual std::vector<Point3D> knnQuery3D(const Point3D& p, int k) = 0;
    virtual void save(const std::string &filename) = 0;
    virtual void load(const std::string &filename) = 0;
    virtual std::string getStats() = 0;
    virtual void flush() = 0;
    virtual void setCacheSize(size_t size) = 0;
    virtual ~SpatialIndex() = default;
};

// === DISK-BASED RTREE INDEX ===
class DiskRTreeIndex : public SpatialIndex {
private:
    std::shared_ptr<RTreeNode> root;
    std::unique_ptr<DiskStorageManager> storage;
    std::string indexDir;
    std::unique_ptr<LRUCache<size_t, std::shared_ptr<DataPage>>> pageCache;
    std::unique_ptr<LRUCache<size_t, std::shared_ptr<RTreeNode>>> nodeCache;
    size_t nextPageId = 0;
    size_t nextNodeId = 0;
    
    // Statistics
    size_t totalPoints2D = 0;
    size_t totalPoints3D = 0;
    size_t totalPolygons = 0;
    size_t diskReads = 0;
    size_t diskWrites = 0;
    size_t cacheHits = 0;
    size_t cacheMisses = 0;
    
    std::shared_ptr<DataPage> loadPage(size_t pageId) {
        std::shared_ptr<DataPage> page;
        
        // Check cache
        if (pageCache->get(pageId, page)) {
            cacheHits++;
            return page;
        }
        
        cacheMisses++;
        diskReads++;
        
        // Load from disk
        std::string data = storage->loadPage(pageId);
        if (data.empty()) {
            return nullptr;
        }
        
        page = std::make_shared<DataPage>();
        page->pageId = pageId;
        page->deserialize(data);
        
        // Add to cache
        pageCache->put(pageId, page);
        
        return page;
    }
    
    void savePage(std::shared_ptr<DataPage> page) {
        if (!page->dirty) return;
        
        diskWrites++;
        storage->savePage(page->pageId, page->serialize());
        page->dirty = false;
    }
    
    std::shared_ptr<RTreeNode> loadNode(size_t nodeId) {
        std::shared_ptr<RTreeNode> node;
        
        // Check cache
        if (nodeCache->get(nodeId, node)) {
            cacheHits++;
            return node;
        }
        
        cacheMisses++;
        diskReads++;
        
        // Load from disk
        std::string data = storage->loadPage(nodeId + 1000000); // Offset for nodes
        if (data.empty()) {
            return nullptr;
        }
        
        node = std::make_shared<RTreeNode>();
        std::unordered_map<size_t, std::shared_ptr<RTreeNode>> emptyMap;
        node->deserialize(data, emptyMap);
        
        // Add to cache
        nodeCache->put(nodeId, node);
        
        return node;
    }
    
    void saveNode(std::shared_ptr<RTreeNode> node) {
        if (!node->dirty) return;
        
        diskWrites++;
        storage->savePage(node->nodeId + 1000000, node->serialize());
        node->dirty = false;
    }
    
    std::shared_ptr<RTreeNode> chooseLeaf(std::shared_ptr<RTreeNode> node, const Rectangle& mbr) {
        while (!node->isLeaf) {
            // Load children if needed
            for (auto& child : node->children) {
                if (!child->children.empty() || child->isLeaf) continue;
                auto loadedChild = loadNode(child->nodeId);
                if (loadedChild) {
                    child = loadedChild;
                }
            }
            
            // Choose best child
            std::shared_ptr<RTreeNode> best = nullptr;
            double minEnlargement = std::numeric_limits<double>::max();
            
            for (auto& child : node->children) {
                double enlargement = child->mbr.enlarge(mbr).area() - child->mbr.area();
                if (enlargement < minEnlargement || 
                    (enlargement == minEnlargement && child->mbr.area() < best->mbr.area())) {
                    minEnlargement = enlargement;
                    best = child;
                }
            }
            
            node = best;
        }
        
        return node;
    }
    
    std::pair<std::shared_ptr<RTreeNode>, std::shared_ptr<RTreeNode>> splitNode(std::shared_ptr<RTreeNode> node) {
        auto newNode = std::make_shared<RTreeNode>();
        newNode->isLeaf = node->isLeaf;
        newNode->nodeId = nextNodeId++;
        
        // Quadratic split
        size_t seed1 = 0, seed2 = 1;
        double maxWaste = 0;
        
        // Find two seeds with maximum waste
        for (size_t i = 0; i < node->children.size(); ++i) {
            for (size_t j = i + 1; j < node->children.size(); ++j) {
                Rectangle combined = node->children[i]->mbr.enlarge(node->children[j]->mbr);
                double waste = combined.area() - node->children[i]->mbr.area() - node->children[j]->mbr.area();
                if (waste > maxWaste) {
                    maxWaste = waste;
                    seed1 = i;
                    seed2 = j;
                }
            }
        }
        
        // Distribute entries
        std::vector<std::shared_ptr<RTreeNode>> group1, group2;
        group1.push_back(node->children[seed1]);
        group2.push_back(node->children[seed2]);
        
        Rectangle mbr1 = node->children[seed1]->mbr;
        Rectangle mbr2 = node->children[seed2]->mbr;
        
        for (size_t i = 0; i < node->children.size(); ++i) {
            if (i == seed1 || i == seed2) continue;
            
            double enlargement1 = mbr1.enlarge(node->children[i]->mbr).area() - mbr1.area();
            double enlargement2 = mbr2.enlarge(node->children[i]->mbr).area() - mbr2.area();
            
            if (enlargement1 < enlargement2 || 
                (enlargement1 == enlargement2 && group1.size() < group2.size())) {
                group1.push_back(node->children[i]);
                mbr1 = mbr1.enlarge(node->children[i]->mbr);
            } else {
                group2.push_back(node->children[i]);
                mbr2 = mbr2.enlarge(node->children[i]->mbr);
            }
        }
        
        // Ensure minimum fill
        while (group1.size() < RTreeNode::MIN_ENTRIES && !group2.empty()) {
            group1.push_back(group2.back());
            group2.pop_back();
        }
        while (group2.size() < RTreeNode::MIN_ENTRIES && !group1.empty()) {
            group2.push_back(group1.back());
            group1.pop_back();
        }
        
        node->children = group1;
        node->updateMBR();
        node->dirty = true;
        
        newNode->children = group2;
        newNode->updateMBR();
        newNode->dirty = true;
        
        return {node, newNode};
    }
    
    std::shared_ptr<RTreeNode> insertWithSplit(std::shared_ptr<RTreeNode> node, 
                                               std::shared_ptr<RTreeNode> entry, 
                                               int level) {
        if (level == 0) {
            // Leaf level
            node->children.push_back(entry);
            node->updateMBR();
            
            if (node->children.size() > RTreeNode::MAX_ENTRIES) {
                auto [n1, n2] = splitNode(node);
                
                if (node == root) {
                    auto newRoot = std::make_shared<RTreeNode>();
                    newRoot->nodeId = nextNodeId++;
                    newRoot->isLeaf = false;
                    newRoot->children.push_back(n1);
                    newRoot->children.push_back(n2);
                    newRoot->updateMBR();
                    newRoot->dirty = true;
                    root = newRoot;
                    return nullptr;
                }
                
                return n2;
            }
            
            return nullptr;
        }
        
        // Choose subtree
        auto best = chooseLeaf(node, entry->mbr);
        auto newNode = insertWithSplit(best, entry, level - 1);
        
        if (newNode) {
            node->children.push_back(newNode);
            node->updateMBR();
            
            if (node->children.size() > RTreeNode::MAX_ENTRIES) {
                auto [n1, n2] = splitNode(node);
                
                if (node == root) {
                    auto newRoot = std::make_shared<RTreeNode>();
                    newRoot->nodeId = nextNodeId++;
                    newRoot->isLeaf = false;
                    newRoot->children.push_back(n1);
                    newRoot->children.push_back(n2);
                    newRoot->updateMBR();
                    newRoot->dirty = true;
                    root = newRoot;
                    return nullptr;
                }
                
                return n2;
            }
        } else {
            node->updateMBR();
        }
        
        return nullptr;
    }
    
    void rangeSearchNode(std::shared_ptr<RTreeNode> node, const Rectangle& window, 
                        std::vector<Point2D>& results2D, std::vector<Point3D>& results3D, 
                        std::vector<Polygon>& resultsPolygon) {
        if (!node->mbr.intersects(window)) return;
        
        if (node->isLeaf) {
            if (node->dataPageId != std::numeric_limits<size_t>::max()) {
                auto page = loadPage(node->dataPageId);
                if (page) {
                    for (const auto& p : page->points2D) {
                        if (window.contains(p)) {
                            results2D.push_back(p);
                        }
                    }
                    
                    for (const auto& p : page->points3D) {
                        Point2D p2d(p.x, p.y);
                        if (window.contains(p2d)) {
                            results3D.push_back(p);
                        }
                    }
                    
                    for (const auto& poly : page->polygons) {
                        if (window.intersects(poly.getBoundingBox())) {
                            resultsPolygon.push_back(poly);
                                        }
                    }
                }
            }
        } else {
            for (auto& child : node->children) {
                if (!child->children.empty() || child->isLeaf) {
                    rangeSearchNode(child, window, results2D, results3D, resultsPolygon);
                } else {
                    auto loadedChild = loadNode(child->nodeId);
                    if (loadedChild) {
                        rangeSearchNode(loadedChild, window, results2D, results3D, resultsPolygon);
                    }
                }
            }
        }
    }
    
public:
    DiskRTreeIndex(const std::string& dir, size_t cacheSize = 100) 
        : indexDir(dir), 
          pageCache(std::make_unique<LRUCache<size_t, std::shared_ptr<DataPage>>>(cacheSize)),
          nodeCache(std::make_unique<LRUCache<size_t, std::shared_ptr<RTreeNode>>>(cacheSize)) {
        storage = std::make_unique<DiskStorageManager>(dir);
        
        // Try to load existing index
        std::string metaFile = dir + "/meta.dat";
        std::ifstream metaIn(metaFile, std::ios::binary);
        if (metaIn.is_open()) {
            // Load metadata
            metaIn.read(reinterpret_cast<char*>(&nextPageId), sizeof(size_t));
            metaIn.read(reinterpret_cast<char*>(&nextNodeId), sizeof(size_t));
            metaIn.read(reinterpret_cast<char*>(&totalPoints2D), sizeof(size_t));
            metaIn.read(reinterpret_cast<char*>(&totalPoints3D), sizeof(size_t));
            metaIn.read(reinterpret_cast<char*>(&totalPolygons), sizeof(size_t));
            
            size_t rootId;
            metaIn.read(reinterpret_cast<char*>(&rootId), sizeof(size_t));
            metaIn.close();
            
            // Load root node
            root = loadNode(rootId);
        }
        
        if (!root) {
            // Create new root
            root = std::make_shared<RTreeNode>();
            root->nodeId = nextNodeId++;
            root->isLeaf = true;
            root->dirty = true;
        }
    }
    
    ~DiskRTreeIndex() {
        flush();
        saveMetadata();
    }
    
    void insert2D(const Point2D& p) override {
        auto page = findOrCreatePageForPoint(p);
        page->points2D.push_back(p);
        page->updateMBR();
        page->dirty = true;
        totalPoints2D++;
        
        savePage(page);
    }
    
    void insert3D(const Point3D& p) override {
        Point2D p2d(p.x, p.y);
        auto page = findOrCreatePageForPoint(p2d);
        page->points3D.push_back(p);
        page->updateMBR();
        page->dirty = true;
        totalPoints3D++;
        
        savePage(page);
    }
    
    void insertPolygon(const Polygon& poly) override {
        Rectangle mbr = poly.getBoundingBox();
        auto page = findOrCreatePageForMBR(mbr);
        page->polygons.push_back(poly);
        page->updateMBR();
        page->dirty = true;
        totalPolygons++;
        
        savePage(page);
    }
    
    std::vector<Point2D> rangeQuery2D(const Rectangle& window) override {
        std::vector<Point2D> results;
        std::vector<Point3D> dummy3D;
        std::vector<Polygon> dummyPoly;
        rangeSearchNode(root, window, results, dummy3D, dummyPoly);
        return results;
    }
    
    std::vector<Point3D> rangeQuery3D(const Rectangle& window) override {
        std::vector<Point2D> dummy2D;
        std::vector<Point3D> results;
        std::vector<Polygon> dummyPoly;
        rangeSearchNode(root, window, dummy2D, results, dummyPoly);
        return results;
    }
    
    std::vector<Polygon> rangeQueryPolygon(const Rectangle& window) override {
        std::vector<Point2D> dummy2D;
        std::vector<Point3D> dummy3D;
        std::vector<Polygon> results;
        rangeSearchNode(root, window, dummy2D, dummy3D, results);
        return results;
    }
    
    std::vector<Point2D> knnQuery2D(const Point2D& p, int k) override {
    struct NodeDist {
        std::shared_ptr<RTreeNode> node;
        double dist;
        bool operator>(const NodeDist& other) const { return dist > other.dist; }
    };
    
    // Comparador personalizado para el par
    struct PointDistComparator {
        bool operator()(const std::pair<double, Point2D>& a, 
                       const std::pair<double, Point2D>& b) const {
            return a.first < b.first;  // max heap basado en distancia
        }
    };
    
    std::priority_queue<NodeDist, std::vector<NodeDist>, std::greater<NodeDist>> pq;
    std::priority_queue<std::pair<double, Point2D>, 
                       std::vector<std::pair<double, Point2D>>, 
                       PointDistComparator> results;
    
    pq.push({root, 0.0});
    
    while (!pq.empty() && static_cast<int>(results.size()) < k) {
        auto current = pq.top();
        pq.pop();
        
        if (current.node->isLeaf) {
            if (current.node->dataPageId != std::numeric_limits<size_t>::max()) {
                auto page = loadPage(current.node->dataPageId);
                if (page) {
                    for (const auto& point : page->points2D) {
                        double d = distance2D(p, point);
                        if (static_cast<int>(results.size()) < k) {
                            results.push({d, point});
                        } else if (d < results.top().first) {
                            results.pop();
                            results.push({d, point});
                        }
                    }
                }
            }
        } else {
            for (auto& child : current.node->children) {
                auto loadedChild = child;
                if (child->children.empty() && !child->isLeaf) {
                    loadedChild = loadNode(child->nodeId);
                }
                
                if (loadedChild) {
                    double minDist = minDistToRectangle(p, loadedChild->mbr);
                    if (static_cast<int>(results.size()) < k || minDist < results.top().first) {
                        pq.push({loadedChild, minDist});
                    }
                }
            }
        }
    }
    
    std::vector<Point2D> finalResults;
    while (!results.empty()) {
        finalResults.push_back(results.top().second);
        results.pop();
    }
    std::reverse(finalResults.begin(), finalResults.end());
    return finalResults;
}

std::vector<Point3D> knnQuery3D(const Point3D& p, int k) override {
    struct NodeDist {
        std::shared_ptr<RTreeNode> node;
        double dist;
        bool operator>(const NodeDist& other) const { return dist > other.dist; }
    };
    
    // Comparador personalizado para el par
    struct PointDistComparator {
        bool operator()(const std::pair<double, Point3D>& a, 
                       const std::pair<double, Point3D>& b) const {
            return a.first < b.first;  // max heap basado en distancia
        }
    };
    
    std::priority_queue<NodeDist, std::vector<NodeDist>, std::greater<NodeDist>> pq;
    std::priority_queue<std::pair<double, Point3D>, 
                       std::vector<std::pair<double, Point3D>>, 
                       PointDistComparator> results;
    
    Point2D p2d(p.x, p.y);
    pq.push({root, 0.0});
    
    while (!pq.empty() && static_cast<int>(results.size()) < k) {
        auto current = pq.top();
        pq.pop();
        
        if (current.node->isLeaf) {
            if (current.node->dataPageId != std::numeric_limits<size_t>::max()) {
                auto page = loadPage(current.node->dataPageId);
                if (page) {
                    for (const auto& point : page->points3D) {
                        double d = distance3D(p, point);
                        if (static_cast<int>(results.size()) < k) {
                            results.push({d, point});
                        } else if (d < results.top().first) {
                            results.pop();
                            results.push({d, point});
                        }
                    }
                }
            }
        } else {
            for (auto& child : current.node->children) {
                auto loadedChild = child;
                if (child->children.empty() && !child->isLeaf) {
                    loadedChild = loadNode(child->nodeId);
                }
                
                if (loadedChild) {
                    double minDist = minDistToRectangle(p2d, loadedChild->mbr);
                    if (static_cast<int>(results.size()) < k || minDist < results.top().first) {
                        pq.push({loadedChild, minDist});
                    }
                }
            }
        }
    }
    
    std::vector<Point3D> finalResults;
    while (!results.empty()) {
        finalResults.push_back(results.top().second);
        results.pop();
    }
    std::reverse(finalResults.begin(), finalResults.end());
    return finalResults;
}

    void save(const std::string& filename) override {
        flush();
        saveMetadata();
        
        // Copy index directory to filename
        std::ofstream out(filename);
        out << indexDir << std::endl;
    }
    
    void load(const std::string& filename) override {
        std::ifstream in(filename);
        std::string dir;
        std::getline(in, dir);
        
        // Reinitialize with loaded directory
        indexDir = dir;
        storage = std::make_unique<DiskStorageManager>(dir);
        
        // Load metadata
        std::string metaFile = dir + "/meta.dat";
        std::ifstream metaIn(metaFile, std::ios::binary);
        if (metaIn.is_open()) {
            metaIn.read(reinterpret_cast<char*>(&nextPageId), sizeof(size_t));
            metaIn.read(reinterpret_cast<char*>(&nextNodeId), sizeof(size_t));
            metaIn.read(reinterpret_cast<char*>(&totalPoints2D), sizeof(size_t));
            metaIn.read(reinterpret_cast<char*>(&totalPoints3D), sizeof(size_t));
            metaIn.read(reinterpret_cast<char*>(&totalPolygons), sizeof(size_t));
            
            size_t rootId;
            metaIn.read(reinterpret_cast<char*>(&rootId), sizeof(size_t));
            metaIn.close();
            
            root = loadNode(rootId);
        }
    }
    
    std::string getStats() override {
        std::ostringstream stats;
        stats << "Disk R-Tree Statistics:\n";
        stats << "Total 2D Points: " << totalPoints2D << "\n";
        stats << "Total 3D Points: " << totalPoints3D << "\n";
        stats << "Total Polygons: " << totalPolygons << "\n";
        stats << "Disk Reads: " << diskReads << "\n";
        stats << "Disk Writes: " << diskWrites << "\n";
        stats << "Cache Hits: " << cacheHits << "\n";
        stats << "Cache Misses: " << cacheMisses << "\n";
        stats << "Cache Hit Rate: " << (cacheHits + cacheMisses > 0 ? 
                (double)cacheHits / (cacheHits + cacheMisses) * 100 : 0) << "%\n";
        stats << "Total Pages: " << nextPageId << "\n";
        stats << "Total Nodes: " << nextNodeId << "\n";
        return stats.str();
    }
    
    void flush() override {
        // Save all dirty pages
        flushNode(root);
        saveMetadata();
    }
    
    void setCacheSize(size_t size) override {
        pageCache->setCapacity(size);
        nodeCache->setCapacity(size);
    }
    
private:
    void flushNode(std::shared_ptr<RTreeNode> node) {
        if (!node) return;
        
        saveNode(node);
        
        if (!node->isLeaf) {
            for (auto& child : node->children) {
                if (!child->children.empty() || child->isLeaf) {
                    flushNode(child);
                }
            }
        }
    }
    
    void saveMetadata() {
        std::string metaFile = indexDir + "/meta.dat";
        std::ofstream out(metaFile, std::ios::binary);
        
        out.write(reinterpret_cast<const char*>(&nextPageId), sizeof(size_t));
        out.write(reinterpret_cast<const char*>(&nextNodeId), sizeof(size_t));
        out.write(reinterpret_cast<const char*>(&totalPoints2D), sizeof(size_t));
        out.write(reinterpret_cast<const char*>(&totalPoints3D), sizeof(size_t));
        out.write(reinterpret_cast<const char*>(&totalPolygons), sizeof(size_t));
        
        size_t rootId = root ? root->nodeId : 0;
        out.write(reinterpret_cast<const char*>(&rootId), sizeof(size_t));
    }
    
    std::shared_ptr<DataPage> findOrCreatePageForPoint(const Point2D& p) {
        // Simple strategy: find leaf node and use its page
        auto leaf = chooseLeaf(root, Rectangle(p.x, p.y, p.x, p.y));
        
        if (leaf->dataPageId == std::numeric_limits<size_t>::max()) {
            // Create new page
            auto page = std::make_shared<DataPage>();
            page->pageId = nextPageId++;
            page->dirty = true;
            
            leaf->dataPageId = page->pageId;
            leaf->dirty = true;
            
            return page;
        }
        
        return loadPage(leaf->dataPageId);
    }
    
    std::shared_ptr<DataPage> findOrCreatePageForMBR(const Rectangle& mbr) {
        auto leaf = chooseLeaf(root, mbr);
        
        if (leaf->dataPageId == std::numeric_limits<size_t>::max()) {
            auto page = std::make_shared<DataPage>();
            page->pageId = nextPageId++;
            page->dirty = true;
            
            leaf->dataPageId = page->pageId;
            leaf->dirty = true;
            
            return page;
        }
        
        return loadPage(leaf->dataPageId);
    }
    
    double minDistToRectangle(const Point2D& p, const Rectangle& r) {
        double dx = 0, dy = 0;
        
        if (p.x < r.x1) dx = r.x1 - p.x;
        else if (p.x > r.x2) dx = p.x - r.x2;
        
        if (p.y < r.y1) dy = r.y1 - p.y;
        else if (p.y > r.y2) dy = p.y - r.y2;
        
        return std::sqrt(dx * dx + dy * dy);
    }
};

// === PYTHON BINDINGS ===
PYBIND11_MODULE(spatialcpp, m){
    m.doc() = "Disk-based spatial index with caching";
    
    // Point2D
    py::class_<Point2D>(m, "Point2D")
        .def(py::init<>())
        .def(py::init<double, double>())
        .def_readwrite("x", &Point2D::x)
        .def_readwrite("y", &Point2D::y);
    
    // Point3D
    py::class_<Point3D>(m, "Point3D")
        .def(py::init<>())
        .def(py::init<double, double, double>())
        .def_readwrite("x", &Point3D::x)
        .def_readwrite("y", &Point3D::y)
        .def_readwrite("z", &Point3D::z);
    
    // Rectangle
    py::class_<Rectangle>(m, "Rectangle")
        .def(py::init<>())
        .def(py::init<double, double, double, double>())
        .def_readwrite("x1", &Rectangle::x1)
        .def_readwrite("y1", &Rectangle::y1)
        .def_readwrite("x2", &Rectangle::x2)
        .def_readwrite("y2", &Rectangle::y2)
        .def("contains", &Rectangle::contains)
        .def("intersects", &Rectangle::intersects)
        .def("area", &Rectangle::area);
    
    // Polygon
    py::class_<Polygon>(m, "Polygon")
        .def(py::init<>())
        .def(py::init<const std::vector<Point2D>&>())
                .def_readwrite("vertices", &Polygon::vertices)
        .def("getBoundingBox", &Polygon::getBoundingBox)
        .def("contains", &Polygon::contains);
    
    // SpatialIndex (abstract base)
    py::class_<SpatialIndex, std::shared_ptr<SpatialIndex>>(m, "SpatialIndex");
    
    // DiskRTreeIndex
    py::class_<DiskRTreeIndex, SpatialIndex, std::shared_ptr<DiskRTreeIndex>>(m, "DiskRTreeIndex")
        .def(py::init<const std::string&, size_t>(), py::arg("directory"), py::arg("cache_size") = 100)
        .def("insert2D", &DiskRTreeIndex::insert2D)
        .def("insert3D", &DiskRTreeIndex::insert3D)
        .def("insertPolygon", &DiskRTreeIndex::insertPolygon)
        .def("rangeQuery2D", &DiskRTreeIndex::rangeQuery2D)
        .def("rangeQuery3D", &DiskRTreeIndex::rangeQuery3D)
        .def("rangeQueryPolygon", &DiskRTreeIndex::rangeQueryPolygon)
        .def("knnQuery2D", &DiskRTreeIndex::knnQuery2D)
        .def("knnQuery3D", &DiskRTreeIndex::knnQuery3D)
        .def("save", &DiskRTreeIndex::save)
        .def("load", &DiskRTreeIndex::load)
        .def("getStats", &DiskRTreeIndex::getStats)
        .def("flush", &DiskRTreeIndex::flush)
        .def("setCacheSize", &DiskRTreeIndex::setCacheSize);
    
    // Utility functions
    m.def("distance2D", &distance2D, "Calculate 2D Euclidean distance");
    m.def("distance3D", &distance3D, "Calculate 3D Euclidean distance");
}