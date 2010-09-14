try:
    from setuptools import Extension, setup
except ImportError:
    from distutils.core import Extension, setup

import os
import sys
import os.path
import platform

def read(name):
    return open(os.path.join(os.path.dirname(__file__), name)).read()

if "posix" not in os.name:
    print "Are you really running a posix compliant OS ?"
    print "Be posix compliant is mandatory"
    sys.exit(1)

if "Linux" != platform.system():
    print "sorry support linux only."
    sys.exit(1)

library_dirs=['/usr/local/lib']
include_dirs=[]


setup(name='pydrizzle',
    version="0.1dev",
    description="",
    long_description=read('README.rst'),
    author='yutaka matsubara',
    author_email='yutaka.matsubara@gmail.com',
    url='http://github.com/mopemope/pydrizzle',
    license='',
    platforms='',
    #packages= ['pydrizzle'],
    install_requires=[
    ],
    
    entry_points="""

    """,
    ext_modules = [
        Extension('_pydrizzle',
            sources=['src/pydrizzle.c', 'src/connection.c'],
                include_dirs=include_dirs,
                library_dirs=library_dirs,
                libraries=["drizzle"],
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


