#!/bin/bash

# Script de compilación para SpatialCPP
echo "=== Compilando SpatialCPP ==="

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Nombre del módulo (debe coincidir con PYBIND11_MODULE)
MODULE_NAME="spatialcpp"

# Verificar pybind11
python3 -c "import pybind11" 2>/dev/null
if [ $? -ne 0 ]; then
    echo -e "${YELLOW}Instalando pybind11...${NC}"
    pip install pybind11
fi

# Limpiar compilaciones anteriores
echo -e "${YELLOW}Limpiando compilaciones anteriores...${NC}"
rm -f *.so *.pyd ${MODULE_NAME}*.so
echo -e "${GREEN}✓ Limpieza completada${NC}"

# Verificar el nombre del módulo en el código
echo -e "\n${BLUE}Verificando configuración del módulo...${NC}"
if [ -f "src/spatial_index.cpp" ]; then
    # Buscar la declaración PYBIND11_MODULE
    MODULE_IN_CODE=$(grep -o "PYBIND11_MODULE([^,]*" src/spatial_index.cpp | sed 's/PYBIND11_MODULE(//')
    
    if [ ! -z "$MODULE_IN_CODE" ]; then
        echo -e "Módulo definido en el código: ${GREEN}$MODULE_IN_CODE${NC}"
        
        if [ "$MODULE_IN_CODE" != "$MODULE_NAME" ]; then
            echo -e "${YELLOW}⚠ ADVERTENCIA: El nombre del módulo en el código ($MODULE_IN_CODE) no coincide con el esperado ($MODULE_NAME)${NC}"
            echo -e "${YELLOW}Usando el nombre del código: $MODULE_IN_CODE${NC}"
            MODULE_NAME=$MODULE_IN_CODE
        fi
    else
        echo -e "${RED}✗ No se pudo encontrar PYBIND11_MODULE en el código${NC}"
    fi
fi

# Compilar
echo -e "\n${YELLOW}Compilando módulo '$MODULE_NAME'...${NC}"

# Opción 1: Usar setup.py si existe y está configurado correctamente
if [ -f "setup.py" ] && grep -q "$MODULE_NAME" setup.py 2>/dev/null; then
    echo "Usando setup.py..."
    python3 setup.py build_ext --inplace
    COMPILE_RESULT=$?
else
    # Opción 2: Compilación directa
    echo "Compilando directamente..."
    
    # Obtener la extensión correcta para el sistema
    EXTENSION=$(python3-config --extension-suffix)
    OUTPUT_FILE="${MODULE_NAME}${EXTENSION}"
    
    echo -e "${BLUE}Archivo de salida: $OUTPUT_FILE${NC}"
    
    # Compilar
    c++ -O3 -Wall -shared -std=c++20 -fPIC \
        $(python3 -m pybind11 --includes) \
        src/spatial_index.cpp \
        -o "$OUTPUT_FILE"
    
    COMPILE_RESULT=$?
fi

if [ $COMPILE_RESULT -eq 0 ]; then
    echo -e "\n${GREEN}✓ Compilación exitosa!${NC}"
    
    # Listar archivos .so generados
    echo -e "\n${BLUE}Archivos generados:${NC}"
    ls -la *.so 2>/dev/null || ls -la *.pyd 2>/dev/null
    
    # Verificar que el módulo se puede importar
    echo -e "\n${YELLOW}Verificando el módulo...${NC}"
    python3 -c "import $MODULE_NAME; print(f'✓ Módulo {$MODULE_NAME} importado correctamente')"
    
    if [ $? -ne 0 ]; then
        echo -e "\n${RED}✗ Error al importar el módulo${NC}"
        echo -e "${YELLOW}Posibles causas:${NC}"
        echo "1. El nombre en PYBIND11_MODULE no coincide con el archivo generado"
        echo "2. Hay un error en el código de inicialización del módulo"
        echo "3. Faltan símbolos o dependencias"
        
        echo -e "\n${YELLOW}Intentando diagnosticar...${NC}"
        python3 -c "
import os
print('Archivos .so en el directorio:')
for f in os.listdir('.'):
    if f.endswith('.so') or f.endswith('.pyd'):
        print(f'  - {f}')
"
        
        # Intentar importar con el nombre del archivo
        SO_FILE=$(ls *.so 2>/dev/null | head -n1)
        if [ ! -z "$SO_FILE" ]; then
            MODULE_FROM_FILE=$(basename "$SO_FILE" .so | sed 's/\.cpython.*//')
            echo -e "\n${YELLOW}Intentando importar como '$MODULE_FROM_FILE'...${NC}"
            python3 -c "import $MODULE_FROM_FILE; print(f'✓ Funciona como {$MODULE_FROM_FILE}!')" 2>/dev/null
            
            if [ $? -eq 0 ]; then
                echo -e "\n${GREEN}El módulo funciona pero con un nombre diferente.${NC}"
                echo -e "${YELLOW}Debes cambiar PYBIND11_MODULE($MODULE_FROM_FILE, m) en el código${NC}"
                echo -e "${YELLOW}O importarlo como: import $MODULE_FROM_FILE${NC}"
            fi
        fi
    else
        # Mostrar información del módulo si se importó correctamente
        echo -e "\n${GREEN}Información del módulo:${NC}"
        python3 -c "
import $MODULE_NAME
print(f'Nombre: {$MODULE_NAME.__name__}')
if hasattr($MODULE_NAME, '__file__'):
    print(f'Ubicación: {$MODULE_NAME.__file__}')
print('\\nContenido:')
for item in dir($MODULE_NAME):
    if not item.startswith('_'):
        print(f'  - {item}')
"
    fi
else
    echo -e "${RED}✗ Error en la compilación${NC}"
    exit 1
fi