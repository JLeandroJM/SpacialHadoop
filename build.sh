#!/usr/bin/env bash
set -e

echo "[+] Compilando spatialcpp con pybind11, RTree+Grid, KNN, Join..."
```bash
c++ -O3 -Wall -shared -std=c++20 -fPIC \
  $(python3 -m pybind11 --includes) \
  spatialcpp.cpp -o spatialcpp$(python3-config --extension-suffix)
```

echo "[✓] Compilado exitosamente"
