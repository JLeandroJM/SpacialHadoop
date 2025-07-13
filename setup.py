from setuptools import setup, Extension
import os

try:
    from pybind11.setup_helpers import Pybind11Extension, build_ext
    import pybind11
except ImportError:
    print("Error: pybind11 no está instalado.")
    print("Instálalo con: pip install pybind11")
    exit(1)

ext_modules = [
    Pybind11Extension(
        "spatialcpp",
        ["src/spatial_index.cpp"],
        include_dirs=[
            # Incluir el directorio src
            "src",
            # Incluir headers de pybind11
            pybind11.get_include(),
        ],
        cxx_std=17,
        extra_compile_args=['-O3', '-Wall'],
    ),
]

setup(
    name="spatialcpp",
    version="1.0.0",
    author="SpatialCPP Team",
    description="High-performance spatial indexing library",
    long_description=open("README.md").read() if os.path.exists("README.md") else "",
    long_description_content_type="text/markdown",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.6",
    install_requires=[
        "numpy>=1.19.0",
        "pybind11>=2.6.0"
    ],
)