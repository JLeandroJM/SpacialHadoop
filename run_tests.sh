#!/bin/bash

# Script para ejecutar tests de SpatialCPP
echo "🧪 Ejecutando tests de SpatialCPP..."

# Verificar que el módulo esté compilado
python3 -c "import spatialcpp" 2>/dev/null
if [ $? -ne 0 ]; then
    echo "⚠️  El módulo no está compilado. Compilando..."
    ./compile.sh || exit 1
fi

# Ejecutar tests
if [ -f "tests/test_spatial_index.py" ]; then
    echo "Ejecutando test_spatial_index.py..."
    python3 tests/test_spatial_index.py
else
    echo "✗ No se encuentra tests/test_spatial_index.py"
    exit 1
fi

echo "✓ Tests completados"