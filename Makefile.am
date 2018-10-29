AM_CPPFLAGS = -iquote $(top_builddir) -iquote $(top_srcdir) -iquote $(top_srcdir)/cwds

SUBDIRS =

noinst_LTLIBRARIES = libsockettask.la

SOURCES = \
	AIEndPoint.h \
	AIEndPoint.cxx \
	ConnectToEndPoint.h \
	ConnectToEndPoint.cxx

libsockettask_la_SOURCES = ${SOURCES}
libsockettask_la_CXXFLAGS = @LIBCWD_R_FLAGS@
libsockettask_la_LIBADD = @LIBCWD_R_LIBS@

# --------------- Maintainer's Section

if MAINTAINER_MODE
distclean-local:
	-rm -f *.s *.ii

clean-local:
endif

MAINTAINERCLEANFILES = $(srcdir)/Makefile.in
