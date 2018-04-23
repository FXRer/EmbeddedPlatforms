## $Id: Makefile.am,v 1.1 2000/08/21 22:04:59 skr Exp $

## Process this file with automake to produce Makefile.in 

EXTRA_PROGRAMS = gcanterm
bin_PROGRAMS = caracadim

caracadim_SOURCES = caracadim.c
caracadim_DEPENDENCIES = libcaraca_client.la ../can_proto.o ../sock_proto.o
caracadim_LDADD = libcaraca_client.la ../can_proto.o ../sock_proto.o


## libraries
caracaincludedir = $(includedir)/caraca

lib_LTLIBRARIES = libcaraca_client.la
libcaraca_client_la_SOURCES = caraca_client.c
caracainclude_HEADERS = caraca_client.h

CLEANFILES = *~
