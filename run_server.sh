#!/bin/bash

echo "🚀 Iniciando servidor SpatialCPP con API..."

# Verificar que el módulo está compilado
if [ ! -f "spatialcpp*.so" ]; then
    echo "⚠️  Módulo no encontrado. Compilando..."
    ./compile.sh || exit 1
fi

# Instalar Flask si no está
pip install flask flask-cors 2>/dev/null

# Ejecutar servidor con PYTHONPATH correcto
PYTHONPATH=$(pwd) python3 server.py