## Process this file with automake to produce Makefile.in

SUBDIRS = src po data

EXTRA_DIST = config.rpath mkinstalldirs

CLEANFILES = *~

ACLOCAL_AMFLAGS = -I /usr/share/aclocal

dist-hook:
	rm -f AUTHORS ChangeLog
	ln -s data/doc/AUTHORS
	ln -s data/doc/ChangeLog
