#!/bin/bash

# Script de limpieza para SpatialCPP
echo "🧹 Limpiando el proyecto SpatialCPP..."

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Función de limpieza
clean_files() {
    local pattern=$1
    local description=$2
    if ls $pattern 1> /dev/null 2>&1; then
        rm -f $pattern
        echo -e "${GREEN}✓${NC} Eliminados: $description"
    fi
}

clean_dirs() {
    local pattern=$1
    local description=$2
    if [ -d "$pattern" ] || ls -d $pattern 1> /dev/null 2>&1; then
        rm -rf $pattern
        echo -e "${GREEN}✓${NC} Eliminados: $description"
    fi
}

echo -e "\n${YELLOW}Limpiando archivos compilados...${NC}"
clean_files "*.so" "Bibliotecas compartidas (.so)"
clean_files "*.pyd" "Extensiones Python Windows (.pyd)"
clean_files "*.dll" "Bibliotecas Windows (.dll)"
clean_files "spatialcpp*.so" "Módulos spatialcpp"
clean_files "*.o" "Archivos objeto (.o)"

echo -e "\n${YELLOW}Limpiando directorios de build...${NC}"
clean_dirs "build/" "Directorio build"
clean_dirs "dist/" "Directorio dist"
clean_dirs "*.egg-info/" "Información de paquete egg"
clean_dirs "__pycache__/" "Cache de Python"
clean_dirs ".pytest_cache/" "Cache de pytest"

echo -e "\n${YELLOW}Limpiando archivos Python compilados...${NC}"
find . -name "*.pyc" -delete 2>/dev/null && echo -e "${GREEN}✓${NC} Eliminados: Archivos .pyc"
find . -name "__pycache__" -type d -delete 2>/dev/null && echo -e "${GREEN}✓${NC} Eliminados: Directorios __pycache__"

echo -e "\n${YELLOW}Limpiando archivos temporales...${NC}"
clean_files "*~" "Archivos de respaldo (~)"
clean_files "*.tmp" "Archivos temporales"
clean_files ".DS_Store" "Archivos DS_Store (macOS)"

echo -e "\n${YELLOW}Limpiando archivos de test...${NC}"
clean_files "test_index*" "Índices de prueba"
clean_files "*.dat" "Archivos de datos de prueba"

# Desinstalar el paquete si está instalado
echo -e "\n${YELLOW}Desinstalando paquete spatialcpp si existe...${NC}"
pip uninstall spatialcpp -y 2>/dev/null && echo -e "${GREEN}✓${NC} Paquete desinstalado" || echo -e "${YELLOW}⚠${NC} Paquete no estaba instalado"

echo -e "\n${GREEN}✨ Limpieza completada!${NC}"
echo -e "${YELLOW}El proyecto está listo para una compilación limpia.${NC}"