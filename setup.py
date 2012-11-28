try:
    from setuptools import Extension, setup
except ImportError:
    from distutils.core import Extension, setup

import os
import sys
import os.path
import platform
import fnmatch
import sys
import os.path
import platform

develop = False
develop = True 

def read(name):
    return open(os.path.join(os.path.dirname(__file__), name)).read()

def check_platform():
    if "posix" not in os.name:
        print("Are you really running a posix compliant OS ?")
        print("Be posix compliant is mandatory")
        sys.exit(1)

def get_sources(path, ignore_files):
    src = []
    for root, dirs , files in os.walk(path):
        for file in files:
            src_path = os.path.join(root, file)
            #ignore = reduce(lambda x, y: x or y, [fnmatch.fnmatch(src_path, i) for i in ignore_files])
            ignore = [i for i in ignore_files if  fnmatch.fnmatch(src_path, i)]
            if not ignore and src_path.endswith(".c"):
                src.append(src_path)
    return src

define_macros = [] 
install_requires = []

sources = get_sources("src", [""])

library_dirs=[]
include_dirs=["/usr/include/libdrizzle-1.0", "/usr/include/libdrizzle-1.0/libdrizzle"]


define_macros.append(("DEVELOP",None))

setup(name='pydrizzle',
    version="0.1dev",
    description="",
    long_description=read('README.rst'),
    author='yutaka matsubara',
    author_email='yutaka.matsubara@gmail.com',
    url='http://github.com/mopemope/pydrizzle',
    license='',
    platforms='',
    packages= ['pydrizzle'],
    install_requires=install_requires,
    
    entry_points="""

    """,
    ext_modules = [
        Extension('_pydrizzle',
                sources=sources,
                include_dirs=include_dirs,
                library_dirs=library_dirs,
                libraries=["drizzle"],
                define_macros=define_macros
                #extra_compile_args=["-DDEBUG"],
            )],

    classifiers=[
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: C',
        'Programming Language :: Python',
    ],
)


