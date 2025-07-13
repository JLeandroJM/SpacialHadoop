# SpatialCPP - Biblioteca de Indexación Espacial de Alto Rendimiento

## 📋 Descripción del Proyecto

SpatialCPP es una biblioteca de indexación espacial implementada en C++ con bindings para Python y una interfaz web interactiva. Incluye implementaciones optimizadas de R-Tree y Grid Index para consultas espaciales eficientes.

## 🏗️ Estructura del Proyecto

```
SpatialCPP/
├── src/
│   └── spatial_index.cpp      # Implementación completa + bindings Python
├── tests/
│   └── test_spatial_index.py  # Tests básicos del módulo
├── web/
│   └── index.html             # Interfaz web interactiva
├── setup.py                   # Script de instalación Python
├── build.sh                   # Script de compilación simple
├── compile.sh                 # Script de compilación detallado
├── compile_wasm.sh           # Script para compilación WebAssembly
├── run.sh                    # Script para ejecutar la aplicación
├── start.sh                  # Script principal (TODO: crear)
├── clean.sh                  # Script de limpieza (TODO: crear)
└── README.md                 # Este archivo
```

## 🚀 Inicio Rápido

### Ejecutar con un solo comando:
```bash
./start.sh
```

Esto automáticamente:
1. Limpiará cualquier compilación anterior
2. Instalará las dependencias necesarias
3. Compilará el módulo C++
4. Ejecutará los tests
5. Abrirá la interfaz web


## 📊 Rendimiento

El módulo está optimizado para:
- Inserción rápida de millones de puntos
- Consultas por rango en milisegundos
- KNN queries eficientes
- Bajo uso de memoria con estructuras optimizadas
