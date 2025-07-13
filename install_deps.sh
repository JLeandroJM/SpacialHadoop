#!/bin/bash

# Script para instalar todas las dependencias
echo "📦 Instalando dependencias para SpatialCPP..."

# Colores
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Actualizar pip
echo -e "${YELLOW}Actualizando pip...${NC}"
python3 -m pip install --upgrade pip

# Instalar desde requirements.txt si existe
if [ -f "requirements.txt" ]; then
    echo -e "${YELLOW}Instalando desde requirements.txt...${NC}"
    pip install -r requirements.txt
else
    # Instalar manualmente
    echo -e "${YELLOW}Instalando dependencias manualmente...${NC}"
    pip install pybind11 numpy pytest
fi

echo -e "\n${GREEN}✓ Dependencias instaladas!${NC}"

# Verificar instalación
echo -e "\n${YELLOW}Verificando instalación:${NC}"
python3 -c "import pybind11; print(f'✓ pybind11 {pybind11.__version__}')"
python3 -c "import numpy; print(f'✓ numpy {numpy.__version__}')"
python3 -c "import pytest; print(f'✓ pytest {pytest.__version__}')"