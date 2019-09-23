OBJEXT = o
CXX = g++
CXXDEPMODE = depmode=gcc3
CXXFLAGS = -g -O2 -Wall -D_FILE_OFFSET_BITS=64 -D_FORTIFY_SOURCE=2 -std=c++11
CYGPATH_W = echo
DEFS = -DHAVE_CONFIG_H
DEPDIR = $(SRC_S3)/.deps
MKDIR_P = mkdir

BIN_DIR = ./bin
OBJ_DIR = ./obj
SRC_S3 = ./src/protocols/s3
SRC_FTP = ./src/protocols/ftp
SRC_SMB = ./src/protocols/smb

DEFAULT_INCLUDES = -I. -I$(top_builddir)
DEPS_CFLAGS = -D_FILE_OFFSET_BITS=64 -I/usr/include/fuse -I/usr/include/libxml2
AM_CPPFLAGS = $(DEPS_CFLAGS)
CXXCOMPILE = $(CXX) $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) \
	$(AM_CPPFLAGS) $(CPPFLAGS) $(AM_CXXFLAGS) $(CXXFLAGS)

CXXLD = $(CXX)
CXXLINK = $(CXXLD) $(AM_CXXFLAGS) $(CXXFLAGS) $(AM_LDFLAGS) $(LDFLAGS) \
	-o $(BIN_DIR)/$@

s3fs_OBJECTS = $(SRC_S3)/s3io.$(OBJEXT) $(SRC_S3)/s3fs.$(OBJEXT) $(SRC_S3)/curl.$(OBJEXT) $(SRC_S3)/cache.$(OBJEXT) \
	$(SRC_S3)/string_util.$(OBJEXT) $(SRC_S3)/s3fs_util.$(OBJEXT) $(SRC_S3)/fdcache.$(OBJEXT) \
	$(SRC_S3)/common_auth.$(OBJEXT) $(SRC_S3)/addhead.$(OBJEXT) $(SRC_S3)/openssl_auth.$(OBJEXT)

ftp_OBJECTS = $(SRC_FTP)/ftpio.$(OBJEXT)
smb_OBJECTS = $(SRC_SMB)/smbio.$(OBJEXT)
multifs_OBJECTS = ./src/main.$(OBJEXT)
multifs_test_OBJECTS = ./src/multifs-test.$(OBJEXT)
all_OBJECTS = $(OBJ_DIR)/addhead.$(OBJEXT) $(OBJ_DIR)/cache.$(OBJEXT) $(OBJ_DIR)/common_auth.$(OBJEXT) $(OBJ_DIR)/curl.$(OBJEXT) \
	$(OBJ_DIR)/fdcache.$(OBJEXT) $(OBJ_DIR)/ftpio.$(OBJEXT) $(OBJ_DIR)/main.$(OBJEXT) $(OBJ_DIR)/openssl_auth.$(OBJEXT) \
	$(OBJ_DIR)/s3fs.$(OBJEXT) $(OBJ_DIR)/s3fs_util.$(OBJEXT) $(OBJ_DIR)/s3io.$(OBJEXT) $(OBJ_DIR)/smbio.$(OBJEXT) \
	$(OBJ_DIR)/string_util.$(OBJEXT)

LDADD= -lfuse -pthread -lcurl -lxml2 -lcrypto
test_string_util_LDADD = $(LDADD)

PROGRAMS = s3 ftp smb multifs multifs-test

all: Makefile $(PROGRAMS)

mkdir-tmp: 
	#-rm -rf $(DEPDIR)
	-$(MKDIR_P) $(DEPDIR)
	-$(MKDIR_P) $(BIN_DIR)
	-$(MKDIR_P) $(OBJ_DIR)

s3: mkdir-tmp $(s3fs_OBJECTS)

ftp: $(ftp_OBJECTS)
smb: $(smb_OBJECTS)

multifs: $(multifs_OBJECTS)
	$(AM_V_CXXLD)$(CXXLINK) $(all_OBJECTS) $(LDADD) $(LIBS)

multifs-test: $(multifs_test_OBJECTS)
	$(AM_V_CXXLD)$(CXXLINK) $(OBJ_DIR)/multifs-test.$(OBJEXT)

.cpp.o:
	$(AM_V_CXX)$(CXXCOMPILE) -MT $@ -MD -MP -MF $*.Tpo -c -o $@ $<
	$(AM_V_at) mv -f $*.Tpo $(DEPDIR)/
	cp -f $@ $(OBJ_DIR)/

clean: clean-am

clean-am: clean-binPROGRAMS clean-noinstPROGRAMS mostlyclean-am 

mostlyclean-am: 
	-rm -f $(OBJ_DIR)/*.$(OBJEXT)
	-rm -f $(SRC_S3)/*.$(OBJEXT)
	-rm -f $(SRC_FTP)/*.$(OBJEXT)
	-rm -f $(SRC_SMB)/*.$(OBJEXT)
	-rm -f src/*.$(OBJEXT)
	-rm -f $(SRC_S3)/*.Tpo
	-rm -rf $(BIN_DIR)/*
	-rm -rf $(DEPDIR)

clean-binPROGRAMS:
	-test -z "$(bin_PROGRAMS)" || rm -f $(bin_PROGRAMS)

clean-noinstPROGRAMS:
	-test -z "$(noinst_PROGRAMS)" || rm -f $(noinst_PROGRAMS)

.MAKE: check-am

.PHONY: CTAGS GTAGS TAGS all all-am check check-TESTS check-am clean \
	clean-binPROGRAMS clean-generic clean-noinstPROGRAMS \
	cscopelist-am ctags ctags-am distclean distclean-compile \
	distclean-generic distclean-tags distdir dvi dvi-am html \
	html-am info info-am maintainer-clean \
	maintainer-clean-generic mostlyclean mostlyclean-compile \
	mostlyclean-generic pdf pdf-am ps ps-am recheck tags \

.PRECIOUS: Makefile


clang-tidy:
	clang-tidy $(s3fs_SOURCES) -- $(DEPS_CFLAGS) $(CPPFLAGS)

# Tell versions [3.59,3.63) of GNU make to not export all variables.
# Otherwise a system limit (for SysV at least) may be exceeded.
.NOEXPORT:
