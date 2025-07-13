## 🚀 Guía Completa de SpatialCPP

### 📁 Estructura del Proyecto

```
SpatialCPP/
├── src/
│   └── spatial_index.cpp          # Implementación completa del índice espacial + bindings Python
├── tests/
│   └── test_spatial_index.py      # Script de pruebas del módulo
├── web/
│   └── index.html                 # Interfaz web interactiva
├── server.py                      # Servidor Flask con API REST
├── setup.py                       # Script de instalación de Python
├── compile.sh                     # Script de compilación
├── clean.sh                       # Script de limpieza
├── start.sh                       # Script principal de inicio
├── run_server.sh                  # Script para ejecutar el servidor
├── requirements.txt               # Dependencias de Python
└── README.md                      # Documentación del proyecto
```

### 📋 Archivos Principales y su Función

1. **`src/spatial_index.cpp`**
   - Implementación del R-Tree en C++
   - Bindings de Python con pybind11
   - Clases: DiskRTreeIndex, Point2D, Point3D, Rectangle, Polygon

2. **`server.py`**
   - API REST con Flask
   - Endpoints para todas las operaciones espaciales
   - Conecta la interfaz web con el módulo C++

3. **`web/index.html`**
   - Interfaz web completa
   - Visualización 2D y 3D con Plotly y Three.js
   - Comunicación con el servidor vía API REST

4. **`setup.py`**
   - Configuración para compilar el módulo Python
   - Define las extensiones y dependencias

5. **Scripts de utilidad**:
   - `compile.sh` - Compila el módulo C++
   - `clean.sh` - Limpia archivos compilados
   - `start.sh` - Inicia todo el sistema
   - `run_server.sh` - Ejecuta solo el servidor

### 🛠️ Requisitos del Sistema

```bash
# Dependencias del sistema
- Python 3.6+
- g++ o clang (compilador C++)
- pip (gestor de paquetes Python)

# Bibliotecas Python (en requirements.txt)
- pybind11>=2.6.0
- numpy>=1.19.0
- flask>=2.0.0
- flask-cors>=3.0.0
- pytest>=6.0.0 (opcional, para tests)
```

### 📦 Instalación Paso a Paso

#### 1. **Clonar/Descargar el proyecto**
```bash
cd ..
```

#### 2. **Crear entorno virtual (recomendado)**
```bash
python3 -m venv venv
source venv/bin/activate  # En Windows: venv\Scripts\activate
```

#### 3. **Instalar dependencias**
```bash
pip install -r requirements.txt
```

#### 4. **Compilar el módulo C++**
```bash
chmod +x compile.sh
./compile.sh
```

### 🚀 Poner en Marcha la Aplicación

#### Opción 1: **Inicio Automático (Recomendado)**
```bash
chmod +x start.sh
./start.sh
```

Este script:
- Verifica las dependencias
- Compila el módulo si es necesario
- Ejecuta los tests
- Inicia el servidor
- Abre la interfaz web

#### Opción 2: **Inicio Manual Paso a Paso**

1. **Compilar el módulo**:
   ```bash
   ./compile.sh
   ```

2. **Verificar que funciona**:
   ```bash
   python3 -c "import spatialcpp; print('✓ Módulo cargado')"
   ```

3. **Ejecutar tests (opcional)**:
   ```bash
   python3 tests/test_spatial_index.py
   ```

4. **Iniciar el servidor**:
   ```bash
   python3 server.py
   ```

5. **En otra terminal, servir la interfaz web**:
   ```bash
   cd web
   python3 -m http.server 8000
   ```

6. **Abrir en el navegador**:
   - Servidor API: http://localhost:5000
   - Interfaz web: http://localhost:8000

### 📝 Crear Scripts Faltantes

Si no tienes algunos scripts, aquí están:

#### `requirements.txt`
```txt
pybind11>=2.6.0
numpy>=1.19.0
flask>=2.0.0
flask-cors>=3.0.0
pytest>=6.0.0
```

### 📊 Uso de la Aplicación

1. **Crear un índice**:
   - Selecciona "R-Tree" 
   - Click en "🔧 Crear Índice"

2. **Cargar datos**:
   - **Opción A**: Subir archivo CSV
   - **Opción B**: Usar comandos INSERT

3. **Ejecutar consultas**:
   ```
   RANGE 0 0 50 50        # Búsqueda por rango
   KNN 25 25 5           # 5 vecinos más cercanos
   INSERT 10 20          # Insertar punto 2D
   INSERT 10 20 30       # Insertar punto 3D
   POLYGON 0,0 10,0 10,10 0,10  # Insertar polígono
   STATS                 # Ver estadísticas
   ```

### 🔍 Verificación de Funcionamiento

```bash
# 1. Verificar que el módulo está compilado
ls -la spatialcpp*.so

# 2. Verificar que se puede importar
python3 -c "import spatialcpp; print(dir(spatialcpp))"

# 3. Verificar que el servidor responde
curl http://localhost:5000/

# 4. Verificar la API
curl -X POST http://localhost:5000/api/create_index \
  -H "Content-Type: application/json" \
  -d '{"type":"rtree"}'
```

### 🛑 Solución de Problemas Comunes

1. **ModuleNotFoundError: spatialcpp**
   ```bash
   ./clean.sh
   ./compile.sh
   ```

2. **Puerto 5000/8000 en uso**
   ```bash
   # Cambiar puerto del servidor en server.py
   app.run(port=5001)  # Usar puerto 5001
   
   # Cambiar puerto de la interfaz web
   python3 -m http.server 8001
   ```

3. **Error de permisos en scripts**
   ```bash
   chmod +x *.sh
   ```

### 📚 Estructura de la API REST

```
POST /api/create_index     - Crear nuevo índice
POST /api/execute_query    - Ejecutar consulta SQL-like
GET  /api/get_all_data    - Obtener todos los datos
GET  /api/stats           - Obtener estadísticas
```

¡Con esto tienes todo lo necesario para ejecutar SpatialCPP! Solo necesitas ejecutar `./start.sh` y todo debería funcionar automáticamente.