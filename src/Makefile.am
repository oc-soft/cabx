
bin_PROGRAMS=cabx


cabx_SOURCES=cabx.c \
	cabx_i.c \
	cabx_main.c \
	exe_info_win.c \
	str_conv_win.c \
	number_parser.c \
	str_hash.c \
	name_compression.gpf

cabx_CPPFLAGS=-I$(srcdir)/../include \
	-I$(top_srcdir)/oclib/col/include \
	-I$(top_srcdir)/oclib/csv/include \
	-I$(top_srcdir)/oclib/buffer/include \
	-I$(top_srcdir)/oclib/cstr/include

cabx_LDFLAGS=-static -municode -specs=$(srcdir)/ucrt.specs
cabx_LDADD=-lpathcch -lcabinet \
	$(top_builddir)/oclib/col/src/liboccol.la \
	$(top_builddir)/oclib/csv/src/liboccsv.la \
	$(top_builddir)/oclib/buffer/src/libocbuffer.la \
	$(top_builddir)/oclib/cstr/src/liboccstr.la

.gpf.c:
	$(GPERF) --output-file=$@ $< 
# vi: se ts=4 sw=4 noet:
