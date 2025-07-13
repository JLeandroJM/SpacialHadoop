import spatialcpp
import numpy as np
import time

print("=== Prueba de SpatialCPP ===\n")

# 1. Crear un índice R-Tree en disco
print("1. Creando índice R-Tree...")
try:
    rtree = spatialcpp.DiskRTreeIndex("./test_index", 100)  # capacidad 100
    print("✓ R-Tree creado exitosamente")
except Exception as e:
    print(f"✗ Error creando R-Tree: {e}")
    exit(1)

# 2. Insertar puntos 2D
print("\n2. Insertando puntos 2D...")
points_2d = []
for i in range(10):
    x = np.random.uniform(0, 100)
    y = np.random.uniform(0, 100)
    point = spatialcpp.Point2D(x, y)
    points_2d.append(point)
    rtree.insert2D(point)
    print(f"   Insertado: ({x:.2f}, {y:.2f})")

# 3. Insertar puntos 3D
print("\n3. Insertando puntos 3D...")
points_3d = []
for i in range(5):
    x = np.random.uniform(0, 100)
    y = np.random.uniform(0, 100)
    z = np.random.uniform(0, 50)
    point = spatialcpp.Point3D(x, y, z)
    points_3d.append(point)
    rtree.insert3D(point)
    print(f"   Insertado: ({x:.2f}, {y:.2f}, {z:.2f})")

# 4. Insertar polígonos
print("\n4. Insertando polígonos...")
# Crear un triángulo
vertices = [
    spatialcpp.Point2D(10, 10),
    spatialcpp.Point2D(30, 10),
    spatialcpp.Point2D(20, 30)
]
polygon = spatialcpp.Polygon(vertices)
rtree.insertPolygon(polygon)
print("   Insertado: Triángulo")

# 5. Consulta por rango
print("\n5. Consulta por rango...")
rect = spatialcpp.Rectangle(0, 0, 50, 50)
start = time.time()
results_2d = rtree.rangeQuery2D(rect)
end = time.time()
print(f"   Área de búsqueda: [0,0] - [50,50]")
print(f"   Encontrados {len(results_2d)} puntos 2D en {(end-start)*1000:.2f} ms")
for p in results_2d[:5]:  # Mostrar solo los primeros 5
    print(f"   - ({p.x:.2f}, {p.y:.2f})")
if len(results_2d) > 5:
    print(f"   ... y {len(results_2d)-5} más")

# 6. KNN Query
print("\n6. Búsqueda KNN (3 vecinos más cercanos)...")
query_point = spatialcpp.Point2D(50, 50)
start = time.time()
knn_results = rtree.knnQuery2D(query_point, 3)
end = time.time()
print(f"   Punto de consulta: ({query_point.x}, {query_point.y})")
print(f"   Encontrados {len(knn_results)} vecinos en {(end-start)*1000:.2f} ms")
for p in knn_results:
    dist = spatialcpp.distance2D(query_point, p)
    print(f"   - ({p.x:.2f}, {p.y:.2f}) distancia: {dist:.2f}")

# 7. Estadísticas
print("\n7. Estadísticas del índice:")
stats = rtree.getStats()
print(f"   {stats}")

# 8. Guardar índice
print("\n8. Guardando índice en disco...")
rtree.save("spatial_index.dat")
rtree.flush()
print("✓ Índice guardado exitosamente")

print("\n=== Prueba completada ===")