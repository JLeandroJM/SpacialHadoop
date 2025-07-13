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

#include "GridIndex.hpp"
#include "Index.hpp"
#include "RTree.hpp"

int main() {
    // 1. Definir un conjunto de puntos de ejemplo
   std::vector<Geoname> puntos = {
    {-6.3577f, -78.7999f},
    {-6.3930f, -78.7987f},
    {-6.45528f, -78.799f},
    {-6.6852f, -78.8024f},
    {-6.70405f, -78.7988f},
    {-7.10162f, -78.801f},
    {-7.08333f, -78.8f},
    {-7.07332f, -78.8004f},
    {-6.90934f, -78.8156f}
    // {6.5f, 2.5f},
    // {8.0f, 8.0f},
    // {9.1f, 3.3f},
    // {10.0f, 5.0f},
    // {7.3f, 6.6f}
};


/*


6. 13261340 El Chito [-6.70405, -78.7988]
7. 13239333 El Ingenio [-7.10162, -78.801]
8. 3694056 Paloblanco [-7.08333, -78.8]
9. 3699079 Cajaquis [-7.08333, -78.8]
10. 13239916 Dos de Mayo [-7.07332, -78.8004]
11. 13261346 El Convento [-6.90934, -78.8156]

*/

    // 2. Inicializar el RTree con un máximo de 4 entradas por nodo
    RTreeIndex rtree(4);

    // 3. Construir el árbol usando bulk-load STR
    rtree.build(puntos);


    // 5. Probar k-NN
    float qx = -6.9092f, qy = -78.7987f;
    int k = 10;
    Geoname query{qx, qy};
    std::cout << "Consulta k-NN para punto (" << qx << "," << qy << "), k=" << k << "\n";
    auto vecinos = rtree.kNN(query, k);

    std::cout << "Los " << k << " vecinos mas cercanos son:\n";
    for (int i = 0; i < vecinos.size(); ++i) {
        std::cout << "  " << i+1 << ": ("
                  << vecinos[i].latitude << ", " << vecinos[i].longitude << ")\n";
    }


    return 0;
}