import sys
import os

# Agregar el directorio actual al path
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

try:
    import spatialcpp
    print("✓ Módulo spatialcpp cargado correctamente")
except ImportError as e:
    print(f"Error importando spatialcpp: {e}")
    print("Archivos .so en el directorio:")
    for f in os.listdir('.'):
        if f.endswith('.so'):
            print(f"  - {f}")
    sys.exit(1)

from flask import Flask, request, jsonify, send_from_directory
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

# Estado global
current_index = None
data_points = []
point_id_counter = 1

@app.route('/')
def index():
    return send_from_directory('web', 'index.html')

@app.route('/<path:path>')
def serve_static(path):
    return send_from_directory('web', path)

@app.route('/api/create_index', methods=['POST'])
def create_index():
    global current_index, data_points, point_id_counter
    try:
        data = request.json
        index_type = data.get('type', 'rtree')
        
        if index_type.lower() == 'grid':
            print(index_type)
            current_index = spatialcpp.GridIndex()
        elif index_type.lower() == 'rtree':
            current_index = spatialcpp.RTree()
        data_points = []
        point_id_counter = 1
        
        return jsonify({
            'success': True, 
            'message': f'Índice {index_type} creado exitosamente'
        })
    except Exception as e:
        return jsonify({
            'success': False, 
            'error': str(e)
        })

@app.route('/api/bulk_insert', methods=['POST'])
def bulk_insert():
    global current_index, data_points, point_id_counter
    # Verifica que haya un índice cargado (por ejemplo, creado antes desde /create_index)
    if not current_index:
        return jsonify({
            'success': False,
            'error': 'No hay índice creado'
        }), 400

    try:
        data = request.get_json()

        if 'points' not in data or not isinstance(data['points'], list):
            return jsonify({
                'success': False,
                'error': 'Formato inválido: se esperaba una lista en \"points\"'
            }), 400

        raw_points = data['points']
        point_objs = []

        for p in raw_points:
            if 'x' in p and 'y' in p:
                try:
                    x = float(p['x'])
                    y = float(p['y'])
                    point_objs.append(spatialcpp.Point2D(x, y))
                    point_data = {
                        'id': point_id_counter, 
                        'x': x, 
                        'y': y, 
                        'z': None,
                        'type': '2D'
                    }
                    point_id_counter += 1

                    data_points.append(point_data)
                except ValueError:
                    continue  # omitir puntos mal formateados
            else:
                continue  # omitir puntos incompletos
        
        print(data_points)

        if not point_objs:
            return jsonify({
                'success': False,
                'error': 'No se pudo cargar ningún punto válido'
            }), 400

        current_index.insert2D(point_objs)

        return jsonify({
            'success': True,
            'inserted': len(point_objs)
        })

    except Exception as e:
        return jsonify({
            'success': False,
            'error': str(e)
        }), 500


@app.route('/api/execute_query', methods=['POST'])
def execute_query():
    global current_index, data_points, point_id_counter
    
    try:
        if not current_index:
            return jsonify({
                'success': False, 
                'error': 'No hay índice creado'
            })
        
        data = request.json
        query = data.get('query', '').strip()
        
        if not query:
            return jsonify({
                'success': False, 
                'error': 'Consulta vacía'
            })
        
        parts = query.split()
        command = parts[0].upper()
        
        if command == 'INSERT':
            # INSERT x y [z]
            if len(parts) < 3:
                return jsonify({
                    'success': False, 
                    'error': 'INSERT requiere al menos x, y'
                })
            
            x = float(parts[1])
            y = float(parts[2])
            z = float(parts[3]) if len(parts) > 3 else None
            
            # Insertar en el índice
            if z is not None:

                point = spatialcpp.Point2D(x, y)
                current_index.insert2D([point])
                point_data = {
                    'id': point_id_counter, 
                    'x': x, 
                    'y': y, 
                    'z': None,
                    'type': '2D'
                }
            else:
                point = spatialcpp.Point2D(x, y)
                current_index.insert2D([point])
                point_data = {
                    'id': point_id_counter, 
                    'x': x, 
                    'y': y, 
                    'z': None, 
                    'type': '2D'
                }
            
            data_points.append(point_data)
            point_id_counter += 1
            
            return jsonify({
                'success': True,
                'results': [point_data],
                'message': f'Punto insertado: ({x}, {y}{"" if z is None else f", {z}"})'
            })
        
        elif command == 'RANGE':
            # RANGE x1 y1 x2 y2
            if len(parts) < 5:
                return jsonify({
                    'success': False, 
                    'error': 'RANGE requiere 4 coordenadas'
                })
            
            x1, y1, x2, y2 = map(float, parts[1:5])    
            
            # Consulta en el índice
            results = current_index.rangeQuery2D(x1, y1, x2, y2)
            
            # Convertir resultados a formato JSON
            found_points = []
            for p in results:
                # Buscar el punto en nuestra lista
                for dp in data_points:
                    if dp['type'] in ['2D', '3D'] and abs(dp['x'] - p.x) < 0.001 and abs(dp['y'] - p.y) < 0.001:
                        found_points.append(dp)
                        break
            
            return jsonify({
                'success': True,
                'results': found_points,
                'message': f'Encontrados {len(found_points)} puntos en el rango'
            })
        
        elif command == 'KNN':
            # KNN x y k
            if len(parts) < 4:
                return jsonify({
                    'success': False, 
                    'error': 'KNN requiere x, y, k'
                })
            
            x, y, k = float(parts[1]), float(parts[2]), int(parts[3])
            query_point = spatialcpp.Point2D(x, y)

            print(x, y, k)
            
            # Consulta KNN
            results = current_index.knnQuery2D(query_point, k)

            print("LISTOO")

            for p in results:
                print(f"({p.x}, {p.y})")

            
            # Convertir resultados
            found_points = []
            for p in results:
                dist = spatialcpp.distance2D(query_point, p)
                # Buscar el punto en nuestra lista
                for dp in data_points:
                    if dp['type'] in ['2D', '3D'] and abs(dp['x'] - p.x) < 0.001 and abs(dp['y'] - p.y) < 0.001:
                        point_with_dist = dp.copy()
                        point_with_dist['distance'] = dist
                        found_points.append(point_with_dist)
                        break
            
            return jsonify({
                'success': True,
                'results': found_points,
                'message': f'Encontrados {len(found_points)} vecinos más cercanos'
            })
        
        elif command == 'POLYGON':
            '''
            # POLYGON x1,y1 x2,y2 x3,y3 ...
            coords_str = ' '.join(parts[1:])
            coord_pairs = coords_str.split(',')
            
            if len(coord_pairs) < 3:
                return jsonify({
                    'success': False, 
                    'error': 'POLYGON requiere al menos 3 puntos'
                })
            
            vertices = []
            for i in range(0, len(coord_pairs), 2):
                if i+1 < len(coord_pairs):
                    x = float(coord_pairs[i].strip())
                    y = float(coord_pairs[i+1].strip())
                    vertices.append(spatialcpp.Point2D(x, y))
            
            polygon = spatialcpp.Polygon(vertices)
            current_index.insertPolygon(polygon)
            
            # Calcular centroide para almacenar
            centroid_x = sum(v.x for v in vertices) / len(vertices)
            centroid_y = sum(v.y for v in vertices) / len(vertices)
            
            polygon_data = {
                'id': point_id_counter,
                'x': centroid_x,
                'y': centroid_y,
                'z': None,
                'type': 'Polygon',
                'vertices': [{'x': v.x, 'y': v.y} for v in vertices]
            }
            
            data_points.append(polygon_data)
            point_id_counter += 1
            '''
            
            return jsonify({
                'success': True,
                'results': [],
                'message': f'Polígono insertado con 0 vértices'
            })
        
        else:
            return jsonify({
                'success': False, 
                'error': f'Comando no reconocido: {command}'
            })
        '''
        elif command == 'STATS':
            stats = current_index.getStats()
            return jsonify({
                'success': True,
                'message': f'Estadísticas: {stats}',
                'stats': {
                    'total_points': len(data_points),
                    'points_2d': len([p for p in data_points if p['type'] == '2D']),
                    'points_3d': len([p for p in data_points if p['type'] == '3D']),
                    'polygons': len([p for p in data_points if p['type'] == 'Polygon']),
                    'index_info': stats
                }
            })
        '''
    except Exception as e:
        return jsonify({
            'success': False, 
            'error': f'Error ejecutando consulta: {str(e)}'
        })

@app.route('/api/get_all_data', methods=['GET'])
def get_all_data():
    return jsonify({
        'success': True,
        'data': data_points
    })

if __name__ == '__main__':
    print("=" * 50)
    print("Servidor SpatialCPP con API REST")
    print("=" * 50)
    print("Módulo spatialcpp cargado:", 'spatialcpp' in sys.modules)
    print("Clases disponibles:")
    for attr in dir(spatialcpp):
        if not attr.startswith('_'):
            print(f"  - {attr}")
    print("=" * 50)
    print("Iniciando servidor en http://localhost:5000")
    print("La interfaz web estará disponible en la raíz")
    print("=" * 50)
    
    app.run(debug=True, port=5001, host='0.0.0.0')