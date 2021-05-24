# cython: profile=True
from setuptools import Extension, setup
from Cython.Build import cythonize

# define an extension that will be cythonized and compiled
# ext = Extension(name='cython_test', sources=['helloworld.pyx'])
setup(
    name='cython_test',
    ext_modules=cythonize(['*.pyx'], language_level='3',
                          annotate=True, build_dir='build')
)
