# $Id$
AVRDUDEDIR=../avrdude
PYINC=$(shell python -c "import distutils.sysconfig; print distutils.sysconfig.get_python_inc()")
INCFLAGS=-I. -I$(AVRDUDEDIR) -I$(PYINC)
CC=g++
all: _pydude.so pydude.py

clean: 
	rm _pydude.so pydude.py pydude_wrap.cpp pydude_wrap.os pydude.os

pydude_wrap.cpp pydude.py: pydude.i
	swig -o pydude_wrap.cpp -python -c++ pydude.i

%.os: %.cpp
	$(CC) -Wall -o $@ -c -fPIC $(INCFLAGS) $<


_pydude.so: pydude_wrap.os pydude.os
	$(CC) -Wall -o $@ -shared $* -L$(AVRDUDEDIR) -lavrdude -lusb
