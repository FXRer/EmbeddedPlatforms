# 
# run with python setup.py build_ext -i
#
# needed a patch for distutils in python 2.4
#--- /usr/lib/python2.4/distutils/command/build_ext.py.o 2009-03-01 22:33:33.000000000 +0100
#+++ /usr/lib/python2.4/distutils/command/build_ext.py   2009-03-01 22:37:21.000000000 +0100
#@@ -522,7 +522,7 @@
#         if self.swig_cpp:
#             log.warn("--swig-cpp is deprecated - use --swig-opts=-c++")
#
#-        if self.swig_cpp or ('-c++' in self.swig_opts):
#+        if self.swig_cpp or ('-c++' in self.swig_opts+extension.swig_opts):
#             target_ext = '.cpp'
#         else:
#             target_ext = '.c'
#

from distutils.core import setup, Extension
setup(name='pydude',
      version='0.2',
      description='Python Wrapper for libavrdude',
      author='Axel Wachtler',
      author_email='uracolix@quantentunnel.de',
      url='http://uracoli.nongnu.org',
      ext_modules = [
        Extension("_pydude", 
                  sources=["pydude.i","pydude.cpp"],
                  #extra_sources=["pydude.h","README"],
                  include_dirs = ["../avrdude",'.'],
                  language = "c++",
                  swig_opts=['-c++','-modern',],
                  library_dirs = [".","../avrdude"],
                  libraries = ["avrdude","usb"])
    ],
    py_modules = ["pydude"],
    
    #data_files=[('.', ['pydude.h'])]
)
