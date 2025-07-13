#!/bin/bash

# Script principal para ejecutar SpatialCPP
# Este script maneja todo el proceso de compilación y ejecución

echo "╔══════════════════════════════════════╗"
echo "║      SpatialCPP - Inicio Rápido      ║"
echo "╚══════════════════════════════════════╝"

# Colores
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# Función para verificar comandos
check_command() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}✗ $1 no está instalado${NC}"
        return 1
    else
        echo -e "${GREEN}✓ $1 encontrado${NC}"
        return 0
    fi
}

# Verificar dependencias
echo -e "\n${BLUE}1. Verificando dependencias...${NC}"
DEPS_OK=true

check_command python3 || DEPS_OK=false
check_command g++ || DEPS_OK=false
check_command pip3 || { check_command pip || DEPS_OK=false; }

if [ "$DEPS_OK" = false ]; then
    echo -e "${RED}Por favor instala las dependencias faltantes${NC}"
    exit 1
fi

# Preguntar si limpiar compilaciones anteriores
echo -e "\n${YELLOW}¿Deseas limpiar compilaciones anteriores y datos?${NC}"
echo -e "${YELLOW}Esto eliminará:${NC}"
echo -e "  - Módulos compilados (.so)"
echo -e "  - Archivos de caché"
echo -e "  - Índices guardados"
echo -e "\n${BLUE}¿Limpiar? (s/N):${NC} \c"
read -r CLEAN_RESPONSE

if [[ "$CLEAN_RESPONSE" =~ ^[Ss]$ ]]; then
    echo -e "\n${BLUE}2. Limpiando compilaciones anteriores...${NC}"
    if [ -f "clean.sh" ]; then
        ./clean.sh
    else
        # Limpieza manual si no existe clean.sh
        rm -f *.so *.pyd *.dll spatialcpp*.so
        rm -rf build/ dist/ *.egg-info/
        rm -rf __pycache__/ .pytest_cache/
        find . -name "*.pyc" -delete
        find . -name "__pycache__" -type d -delete
        # Limpiar índices guardados
        rm -rf rtree_index/ spatial_index/ *.dat
    fi
    echo -e "${GREEN}✓ Limpieza completada${NC}"
    NEED_COMPILE=true
else
    echo -e "\n${BLUE}2. Manteniendo archivos existentes${NC}"
    # Verificar si el módulo ya está compilado
    if ls spatialcpp*.so 1> /dev/null 2>&1; then
        echo -e "${GREEN}✓ Módulo compilado encontrado${NC}"
        NEED_COMPILE=false
    else
        echo -e "${YELLOW}⚠ No se encontró módulo compilado, será necesario compilar${NC}"
        NEED_COMPILE=true
    fi
fi

# Instalar dependencias de Python
echo -e "\n${BLUE}3. Verificando dependencias de Python...${NC}"
# Verificar si las dependencias están instaladas
python3 -c "import pybind11" 2>/dev/null
if [ $? -ne 0 ]; then
    echo -e "${YELLOW}Instalando pybind11...${NC}"
    pip3 install pybind11 --quiet
fi

python3 -c "import numpy" 2>/dev/null
if [ $? -ne 0 ]; then
    echo -e "${YELLOW}Instalando numpy...${NC}"
    pip3 install numpy --quiet
fi

python3 -c "import flask" 2>/dev/null
if [ $? -ne 0 ]; then
    echo -e "${YELLOW}Instalando flask y flask-cors...${NC}"
    pip3 install flask flask-cors --quiet
fi

echo -e "${GREEN}✓ Dependencias verificadas${NC}"

# Compilar el módulo solo si es necesario
if [ "$NEED_COMPILE" = true ]; then
    echo -e "\n${BLUE}4. Compilando SpatialCPP...${NC}"
    if [ -f "compile.sh" ]; then
        ./compile.sh
        COMPILE_RESULT=$?
    elif [ -f "setup.py" ]; then
        python3 setup.py build_ext --inplace
        COMPILE_RESULT=$?
    else
        # Compilación directa
        echo "Compilando directamente..."
        c++ -O3 -Wall -shared -std=c++20 -fPIC \
            $(python3 -m pybind11 --includes) \
            src/spatial_index.cpp -o spatialcpp$(python3-config --extension-suffix)

        COMPILE_RESULT=$?
    fi

    if [ $COMPILE_RESULT -eq 0 ]; then
        echo -e "${GREEN}✓ Compilación exitosa${NC}"
    else
        echo -e "${RED}✗ Error en la compilación${NC}"
        exit 1
    fi
else
    echo -e "\n${BLUE}4. Usando módulo compilado existente${NC}"
fi

# Verificar que el módulo se puede importar
echo -e "\n${BLUE}5. Verificando módulo...${NC}"
python3 -c "import spatialcpp; print('✓ Módulo spatialcpp cargado correctamente')"
if [ $? -ne 0 ]; then
    echo -e "${RED}✗ Error al cargar el módulo${NC}"
    echo -e "${YELLOW}Intenta ejecutar con limpieza: ./start.sh${NC}"
    exit 1
fi

# Preguntar qué modo ejecutar
echo -e "\n${BLUE}6. Selecciona el modo de ejecución:${NC}"
echo "1) Solo interfaz web (sin backend)"
echo "2) Servidor completo (backend + interfaz web)"
echo "3) Solo servidor backend"
echo -e "${BLUE}Opción (1-3):${NC} \c"
read -r MODE

case $MODE in
    1)
        # Solo interfaz web
        echo -e "\n${BLUE}Iniciando interfaz web...${NC}"
        cd web 2>/dev/null || { echo -e "${RED}✗ No se encuentra el directorio web${NC}"; exit 1; }
        
        PORT=8000
        while lsof -Pi :$PORT -sTCP:LISTEN -t >/dev/null 2>&1; do
            PORT=$((PORT + 1))
        done
        
        echo -e "${YELLOW}Interfaz web en http://localhost:$PORT${NC}"
        python3 -m http.server $PORT &
        WEB_PID=$!
        
        sleep 2
        
        # Abrir navegador
        if [[ "$OSTYPE" == "darwin"* ]]; then
            open "http://localhost:$PORT"
        elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
            xdg-open "http://localhost:$PORT" 2>/dev/null || echo -e "${YELLOW}Abre http://localhost:$PORT${NC}"
        fi
        
        # Función de limpieza
        cleanup() {
            echo -e "\n${YELLOW}Deteniendo servidor web...${NC}"
            kill $WEB_PID 2>/dev/null
            echo -e "${GREEN}✓ Servidor detenido${NC}"
            exit 0
        }
        
        trap cleanup INT
        wait $WEB_PID
        ;;
        
    2)
        # Servidor completo
        echo -e "\n${BLUE}Iniciando servidor backend...${NC}"
        python3 server.py &
        SERVER_PID=$!
        
        sleep 3
        
        echo -e "\n${BLUE}Iniciando interfaz web...${NC}"
        cd web 2>/dev/null || { echo -e "${RED}✗ No se encuentra el directorio web${NC}"; exit 1; }
        
        PORT=8000
        while lsof -Pi :$PORT -sTCP:LISTEN -t >/dev/null 2>&1; do
            PORT=$((PORT + 1))
        done
        
        python3 -m http.server $PORT &
        WEB_PID=$!
        
        echo -e "\n${GREEN}╔════════════════════════════════════════╗${NC}"
        echo -e "${GREEN}║    SpatialCPP está ejecutándose!       ║${NC}"
        echo -e "${GREEN}║    Backend: http://localhost:5000      ║${NC}"
        echo -e "${GREEN}║    Frontend: http://localhost:$PORT     ║${NC}"
        echo -e "${GREEN}║    Presiona Ctrl+C para detener       ║${NC}"
        echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
        
        sleep 2
        
        # Abrir navegador
        if [[ "$OSTYPE" == "darwin"* ]]; then
            open "http://localhost:$PORT"
        elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
            xdg-open "http://localhost:$PORT" 2>/dev/null
        fi
        
        # Función de limpieza
        cleanup() {
            echo -e "\n${YELLOW}Deteniendo servidores...${NC}"
            kill $SERVER_PID 2>/dev/null
            kill $WEB_PID 2>/dev/null
            cd ..
            echo -e "${GREEN}✓ Servidores detenidos${NC}"
            exit 0
        }
        
        trap cleanup INT
        wait
        ;;
        
    3)
        # Solo backend
        echo -e "\n${BLUE}Iniciando servidor backend...${NC}"
        echo -e "${GREEN}╔════════════════════════════════════════╗${NC}"
        echo -e "${GREEN}║    Servidor API ejecutándose           ║${NC}"
        echo -e "${GREEN}║    URL: http://localhost:5000          ║${NC}"
        echo -e "${GREEN}║    Presiona Ctrl+C para detener       ║${NC}"
        echo -e "${GREEN}╚════════════════════════════════════════╝${NC}"
        
        python3 server.py
        ;;
        
    *)
        echo -e "${RED}Opción inválida${NC}"
        exit 1
        ;;
esac