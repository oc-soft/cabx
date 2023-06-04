
bin_PROGRAMS=cabx
check_PROGRAMS = t-path-0 t-path-1 t-path-2


cabx_SOURCES=cabx.c \
	cabx_i.c \
	cabx_main.c \
	exe_info_win.c \
	str_conv_win.c \
	number_parser.c \
	str_hash.c \
	path.c \
	name_compression.gpf

if MINGW_HOST
cabx_SOURCES+=path_i_win.c
endif

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

t_path_0_SOURCES=t_path_0.c \
	path.c

t_path_0_CPPFLAGS=-I$(top_srcdir)/oclib/buffer/include 

if MINGW_HOST
t_path_0_SOURCES+=path_i_win.c str_conv_win.c
endif

t_path_0_LDFLAGS=-static -specs=$(srcdir)/ucrt.specs
t_path_0_LDADD=$(top_builddir)/oclib/buffer/src/libocbuffer.la 

if MINGW_HOST
t_path_0_LDADD+=-lpathcch
endif

t_path_1_SOURCES=t_path_1.c \
	path.c

t_path_1_CPPFLAGS=-I$(top_srcdir)/oclib/buffer/include 

if MINGW_HOST
t_path_1_SOURCES+=path_i_win.c str_conv_win.c
endif

t_path_1_LDFLAGS=-static -specs=$(srcdir)/ucrt.specs
t_path_1_LDADD=$(top_builddir)/oclib/buffer/src/libocbuffer.la 

if MINGW_HOST
t_path_1_LDADD+=-lpathcch
endif

t_path_2_SOURCES=t_path_2.c \
	path.c

t_path_2_CPPFLAGS=-I$(top_srcdir)/oclib/buffer/include 

if MINGW_HOST
t_path_2_SOURCES+=path_i_win.c str_conv_win.c
endif

t_path_2_LDFLAGS=-static -specs=$(srcdir)/ucrt.specs
t_path_2_LDADD=$(top_builddir)/oclib/buffer/src/libocbuffer.la 

if MINGW_HOST
t_path_2_LDADD+=-lpathcch
endif


TESTS = t-path-1.test t-path-2.test
if MINGW_HOST
TESTS += t-path-3-win.test
endif


TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
	$(top_srcdir)/tap-driver.sh
EXTRA_DIST = $(TESTS)

LOG_COMPILR = sh

.gpf.c:
	$(GPERF) --output-file=$@ $< 
# vi: se ts=4 sw=4 noet:
