from distutils.core import setup, Extension


setup(name='ambulant',
      version='0.1',
      ext_modules=[
        Extension('ambulant',
            ['ambulantmodule.cpp'],
            libraries=['ambulant'],
#            include_dirs=['/usr/local/include'],
#            library_dirs=['../../build-gcc3/src/libambulant/.libs']
        )
      ]
)
