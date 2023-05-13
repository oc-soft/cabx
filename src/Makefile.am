
bin_PROGRAMS=cabx


cabx_SOURCES=cabx.c \
	cabx_i.c \
	cabx_main.c \
	exe_info_win.c \
	str_conv_win.c

cabx_CPPFLAGS=-I$(srcdir)/../include
cabx_LDFLAGS=-static -municode -specs=$(srcdir)/ucrt.specs
cabx_LDADD=-lpathcch
# vi: se ts=4 sw=4 noet:
