#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <cassert>
#include <set>
#include <string>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "GridIndex.hpp"
#include "Index.hpp"
#include "RTree.hpp"

typedef unsigned int  uint;
typedef unsigned char uchar;

namespace py = pybind11;
namespace fs = std::filesystem;


float distance2D(const Geoname& a, const Geoname& b) {
    float dx = a.latitude - b.latitude;
    float dy = a.longitude - b.longitude;
    return std::sqrt(dx * dx + dy * dy);
}

PYBIND11_MODULE(spatialcpp, m) {
    m.doc() = "Python bindings for RTree+ spatial index";

    // Punto 2D
    py::class_<Geoname>(m, "Point2D")
        .def(py::init<>())
        .def(py::init<float, float>(), py::arg("x"), py::arg("y"))
        .def_readwrite("x", &Geoname::latitude)
        .def_readwrite("y", &Geoname::longitude)
        .def("distance_to", &Geoname::distanceTo);

    // Index CLASE ABSTRACT
    
    py::class_<Index, std::shared_ptr<Index>>(m, "Index")
        .def("insert2D", &Index::build)
        .def("rangeQuery2D", &Index::rangeQuery)
        .def("knnQuery2D", &Index::kNN);

    py::class_<RTreeIndex, Index, std::shared_ptr<RTreeIndex>>(m, "RTree")
        .def(py::init<int>(), py::arg("degree") = 8)  // Constructor con grado
        .def("insert2D", &RTreeIndex::build, py::arg("points"))  // Método build
        .def("rangeQuery2D", &RTreeIndex::rangeQuery, py::arg("minLat"), py::arg("minLon"), py::arg("maxLat"), py::arg("maxLon") )  // Método rangeQuery
        .def("knnQuery2D", &RTreeIndex::kNN, py::arg("q"), py::arg("k"));

    py::class_<GridIndex, Index, std::shared_ptr<GridIndex>>(m, "GridIndex")
        .def(py::init<int, int>(), py::arg("gx") = 10, py::arg("gy") = 10)
        .def("insert2D", &GridIndex::build, py::arg("records"))  
        .def("rangeQuery2D", &GridIndex::rangeQuery, py::arg("minLat"), py::arg("minLon"), py::arg("maxLat"), py::arg("maxLon"))  // Método rangeQuery
        .def("knnQuery2D", &GridIndex::kNN, py::arg("q"), py::arg("k"));
        

    // RTree

    // Funciones de utilidad
    //m.def("distance", &Point::distanceTo, "Euclidean distance between two points");
    m.def("distance2D", &distance2D, "Euclidean distance between two 2D points");

}



/*
int main() {
    // 1. Definir un conjunto de puntos de ejemplo
    std::vector<Point> puntos = {
        {1.0f, 2.0f}, {5.0f, 3.0f}, {4.0f, 7.0f}, {2.0f, 8.0f},
        {9.0f, 1.0f}, {8.0f, 6.0f}, {7.0f, 4.0f}, {3.0f, 5.0f},
        {6.0f, 9.0f}, {0.0f, 0.0f}
    };

    // 2. Inicializar el RTree con un máximo de 4 entradas por nodo
    RTree rtree(4);

    // 3. Construir el árbol usando bulk-load STR
    rtree.bulkLoadSTR(puntos);

    // 4. Obtener la raíz y su MBR (para verificar)
    RNode* raiz = rtree.root;
    MBB b = raiz->mbr;
    std::cout << "MBR raiz: ["
              << b.lower.x << ", " << b.lower.y << "] - ["
              << b.upper.x << ", " << b.upper.y << "]" << std::endl;

    // 5. Probar k-NN
    float qx = 10.0f, qy = 10.0f;
    int k = 3;
    Point query{qx, qy};
    std::cout << "Consulta k-NN para punto (" << qx << "," << qy << "), k=" << k << "\n";
    auto vecinos = rtree.knnQuery(query, k);

    std::cout << "Los " << k << " vecinos mas cercanos son:\n";
    for (int i = 0; i < vecinos.size(); ++i) {
        std::cout << "  " << i+1 << ": ("
                  << vecinos[i].x << ", " << vecinos[i].y << ")\n";
    }

    MBB queryWin;
    queryWin.lower = {0.0f, 0.0f};
    queryWin.upper = {6.0f, 6.0f};

    auto ptsInRange = rtree.rangeQuery(queryWin);
    std::cout << "Puntos en el rango [2,2]-[6,6]:\\n";
    for (auto &p : ptsInRange)
        std::cout << p.x << "  " <<p.y<< std::endl;
    std::cout << std::endl;

    return 0;
}
*/