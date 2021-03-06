# Process this with automake to create Makefile.in

# Work around what appears to be a GNU make bug handling MAKEFLAGS
# values defined in terms of make variables, as is the case for CC and
# friends when we are called from the top level Makefile.
AM_MAKEFLAGS = \
	"AR_FLAGS=$(AR_FLAGS)" \
	"CC_FOR_BUILD=$(CC_FOR_BUILD)" \
	"CFLAGS=$(CFLAGS)" \
	"CXXFLAGS=$(CXXFLAGS)" \
	"CFLAGS_FOR_BUILD=$(CFLAGS_FOR_BUILD)" \
	"CFLAGS_FOR_TARGET=$(CFLAGS_FOR_TARGET)" \
	"INSTALL=$(INSTALL)" \
	"INSTALL_DATA=$(INSTALL_DATA)" \
	"INSTALL_PROGRAM=$(INSTALL_PROGRAM)" \
	"INSTALL_SCRIPT=$(INSTALL_SCRIPT)" \
	"JC1FLAGS=$(JC1FLAGS)" \
	"LDFLAGS=$(LDFLAGS)" \
	"LIBCFLAGS=$(LIBCFLAGS)" \
	"LIBCFLAGS_FOR_TARGET=$(LIBCFLAGS_FOR_TARGET)" \
	"MAKE=$(MAKE)" \
	"MAKEINFO=$(MAKEINFO) $(MAKEINFOFLAGS)" \
	"PICFLAG=$(PICFLAG)" \
	"PICFLAG_FOR_TARGET=$(PICFLAG_FOR_TARGET)" \
	"SHELL=$(SHELL)" \
	"exec_prefix=$(exec_prefix)" \
	"infodir=$(infodir)" \
	"libdir=$(libdir)" \
	"prefix=$(prefix)" \
	"AR=$(AR)" \
	"AS=$(AS)" \
	"CC=$(CC)" \
	"CXX=$(CXX)" \
	"LD=$(LD)" \
	"LIBCFLAGS=$(LIBCFLAGS)" \
	"NM=$(NM)" \
	"PICFLAG=$(PICFLAG)" \
	"RANLIB=$(RANLIB)" \
	"DESTDIR=$(DESTDIR)"

AM_CPPFLAGS = -I. -I$(top_srcdir) $(ZINCS) -I$(top_srcdir)/../include

LIBIBERTY = ../libiberty/libiberty.a

bin_PROGRAMS = fastjar grepjar
fastjar_SOURCES = jartool.c dostime.c compress.c pushback.c shift.c jartool.h \
zipfile.h dostime.h compress.h pushback.h shift.h
fastjar_LDADD = $(ZLIBS) $(LIBIBERTY)
fastjar_DEPENDENCIES = $(ZDEPS) $(LIBIBERTY)
fastjar_CPPFLAGS = $(AM_CPPFLAGS) -DWITH_SHIFT_DOWN

grepjar_SOURCES = jargrep.c dostime.c compress.c pushback.c jartool.h \
zipfile.h dostime.h compress.h pushback.h
grepjar_LDADD = $(ZLIBS) $(LIBIBERTY)
grepjar_DEPENDENCIES = $(ZDEPS) $(LIBIBERTY)

AM_CFLAGS = @fastjar_warn_cflags@
AM_MAKEINFOFLAGS = -I $(srcdir)/../gcc/doc/include

TEXINFO_TEX = ../gcc/doc/include/texinfo.tex
info_TEXINFOS = fastjar.texi
fastjar_TEXINFOS = \
	../gcc/doc/include/gcc-common.texi \
	../gcc/doc/include/gpl.texi \
	gcc-vers.texi
man_MANS = fastjar.1 grepjar.1
EXTRA_DIST = $(man_MANS)

BASEVER = $(srcdir)/../gcc/BASE-VER
DEVPHASE = $(srcdir)/../gcc/DEV-PHASE

TEXI2POD = perl $(srcdir)/../contrib/texi2pod.pl
POD2MAN = pod2man --center="GNU" --release="gcc-$(shell cat $(BASEVER))"

.pod.1:
	-($(POD2MAN) --section=1 $< > $(@).T$$$$ && \
		mv -f $(@).T$$$$ $@) || \
		(rm -f $(@).T$$$$ && exit 1)

.INTERMEDIATE: fastjar.pod grepjar.pod

fastjar.pod: $(srcdir)/fastjar.texi
	-$(TEXI2POD) -D fastjar $< > $@

grepjar.pod: $(srcdir)/fastjar.texi
	-$(TEXI2POD) -D grepjar $< > $@

fastjar.1 grepjar.1: $(BASEVER)

# gcc-vers.texi is generated from the version files.
gcc-vers.texi: $(BASEVER) $(DEVPHASE)
	(echo "@set version-GCC $(shell cat $(BASEVER))"; \
	 if [ "$(shell cat $(DEVPHASE))" = "experimental" ]; \
	 then echo "@set DEVELOPMENT"; \
	 else echo "@clear DEVELOPMENT"; \
	 fi) > $@T
	mv -f $@T $@

# GCC LOCAL CHANGE
# The following commands allow us to release tarballs with the man pages
# and info documentation prebuilt.  This feature is enabled via 
# --enable-generated-files-in-srcdir in the configure script.

if GENINSRC
STAMP_GENINSRC = stamp-geninsrc
else
STAMP_GENINSRC =
endif

all-local: $(STAMP_GENINSRC) 

stamp-geninsrc: fastjar.1 grepjar.1 fastjar.info
	-cp -p $(top_builddir)/fastjar.1 $(srcdir)/fastjar.1
	-cp -p $(top_builddir)/grepjar.1 $(srcdir)/grepjar.1
	-cp -p $(top_builddir)/fastjar.info $(srcdir)/fastjar.info
	touch $@

CLEANFILES = stamp-geninsrc fastjar.info
MAINTAINERCLEANFILES = $(srcdir)/fastjar.1 \
                       $(srcdir)/grepjar.1 \
                       $(srcdir)/fastjar.info
