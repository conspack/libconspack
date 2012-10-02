# $Id: Makefile.am,v 1.1.1.1 2002/04/29 03:19:28 rpav Exp $

include $(top_srcdir)/make-extras

SUBDIRS = include test

lib_LTLIBRARIES = libconspack.la
libconspack_la_SOURCES = encode.c decode.c explain.c

bin_PROGRAMS = conspack
conspack_SOURCES = conspack.c
conspack_LDADD = libconspack.la
