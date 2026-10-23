.POSIX:

CONFIGFILE = config.mk
include $(CONFIGFILE)

OS = linux
# Linux:   linux
# Mac OS:  macos
# Windows: windows
include mk/$(OS).mk


LIB_MAJOR = 1
LIB_MINOR = 0
LIB_VERSION = $(LIB_MAJOR).$(LIB_MINOR)
LIB_NAME = utf8


OBJ_PUBLIC_FUZZ =\
	libutf8_decode_mem.o

OBJ_PUBLIC_DONT_FUZZ =\
	libutf8_is_noncharacter.o\
	libutf8_is_surrogate.o\
	libutf8_is_invalid.o\
	libutf8_is_valid.o\
	libutf8_is_continuation.o\
	libutf8_encode_measure.o\
	libutf8_encode.o\
	libutf8_decode.o

OBJ =\
	$(OBJ_PUBLIC_FUZZ)\
	$(OBJ_PUBLIC_DONT_FUZZ)

HDR =\
	libutf8.h\
	common.h

MAN7 =\
	libutf8.7

MAN3 = $(OBJ:.o=.3)\
	LIBUTF8_ULTIMATE_CODEPOINT.3\
	LIBUTF8_REPLACEMENT_CHARACTER_CODEPOINT.3\
	LIBUTF8_REPLACEMENT_CHARACTER_UTF8.3\
	LIBUTF8_REPLACE_INVALID.3\
	LIBUTF8_RETURN_INVALID.3\
	LIBUTF8_ALLOW_31_BITS.3\
	LIBUTF8_ALLOW_2BYTE_NULL.3\
	LIBUTF8_ALLOW_LONG_ENCODING.3

LOBJ = $(OBJ:.o=.lo)
TOBJ = $(OBJ:.o=.to)
TEST = $(OBJ:.o=.t)
FOBJ = $(OBJ_PUBLIC_FUZZ:.o=.fo)
FUZZ = $(OBJ_PUBLIC_FUZZ:.o=.f)


ALL_CPPFLAGS = $(CPPFLAGS) $(FUZZED_CPPFLAGS)
ALL_CFLAGS   = $(CFLAGS)   $(FUZZED_CFLAGS)
ALL_LDFLAGS  = $(LDFLAGS)  $(FUZZED_LDFLAGS)


all: libutf8.a libutf8.$(LIBEXT) $(TEST)
$(OBJ): $(HDR)
$(LOBJ): $(HDR)
$(TOBJ): $(HDR)
$(FOBJ): $(HDR)
$(TEST): $(HDR) libutf8.a
$(FUZZ): $(HDR) libutf8.a

.c.o:
	$(CC) -c -o $@ $< $(ALL_CFLAGS) $(COV_CFLAGS) $(ALL_CPPFLAGS) $(COV_CPPFLAGS)

.c.lo:
	$(CC) -fPIC -c -o $@ $< $(ALL_CFLAGS) $(COV_CFLAGS) $(ALL_CPPFLAGS) $(COV_CPPFLAGS)

.c.to:
	$(CC) -DTEST -c -o $@ $< $(G) $(ALL_CFLAGS) $(ALL_CPPFLAGS) $(COV_CPPFLAGS)

.to.t:
	$(CC) -o $@ $< libutf8.a $(G) $(ALL_LDFLAGS) $(COV_LDFLAGS)

.c.fo:
	$(CC) -DTEST -DFUZZ -c -o $@ $< $(G) $(ALL_CFLAGS) $(ALL_CPPFLAGS) $(FUZZ_CFLAGS) $(FUZZ_CPPFLAGS)

.fo.f:
	$(CC) -o $@ $< libutf8.a $(G) $(ALL_LDFLAGS) $(TEST_LDFLAGS) $(FUZZ_LDFLAGS)

libutf8.a: $(OBJ)
	@rm -f -- $@
	$(AR) rc $@ $(OBJ)
	$(AR) ts $@ > /dev/null

libutf8.$(LIBEXT): $(LOBJ)
	$(CC) $(LIBFLAGS) -o $@ $(LOBJ) $(ALL_LDFLAGS)

check: $(TEST)
	@set -ex;\
	for t in $(TEST); do\
		$(CHECK_PREFIX) ./$$t;\
	done
# Setting CHECK_PREFIX is intended for developers, setting it
# (specially to use valgrind(1)) may limit what the test code
# is able to test

fuzz: $(FUZZ)
	@set -ex;\
	for f in $(FUZZ); do\
		$(FUZZ_PREFIX) ./$$f $(FUZZ_SUFFIX);\
	done

install: libutf8.a libutf8.$(LIBEXT)
	mkdir -p -- "$(DESTDIR)$(PREFIX)/lib"
	mkdir -p -- "$(DESTDIR)$(PREFIX)/include"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man3"
	mkdir -p -- "$(DESTDIR)$(MANPREFIX)/man7"
	cp -- libutf8.a "$(DESTDIR)$(PREFIX)/lib/"
	cp -- libutf8.$(LIBEXT) "$(DESTDIR)$(PREFIX)/lib/libutf8.$(LIBMINOREXT)"
	$(FIX_INSTALL_NAME) "$(DESTDIR)$(PREFIX)/lib/libutf8.$(LIBMINOREXT)"
	ln -sf -- libutf8.$(LIBMINOREXT) "$(DESTDIR)$(PREFIX)/lib/libutf8.$(LIBMAJOREXT)"
	ln -sf -- libutf8.$(LIBMAJOREXT) "$(DESTDIR)$(PREFIX)/lib/libutf8.$(LIBEXT)"
	cp -- libutf8.h "$(DESTDIR)$(PREFIX)/include/"
	cp -P -- $(MAN3) "$(DESTDIR)$(MANPREFIX)/man3/"
	cp -P -- $(MAN7) "$(DESTDIR)$(MANPREFIX)/man7/"

uninstall:
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libutf8.a"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libutf8.$(LIBMAJOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libutf8.$(LIBMINOREXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/lib/libutf8.$(LIBEXT)"
	-rm -f -- "$(DESTDIR)$(PREFIX)/include/libutf8.h"
	-cd -- "$(DESTDIR)$(MANPREFIX)/man3/" && rm -f -- $(MAN3)
	-cd -- "$(DESTDIR)$(MANPREFIX)/man7/" && rm -f -- $(MAN7)

clean:
	-rm -f -- *.o *.a *.lo *.su *.so *.so.* *.dll *.dylib
	-rm -f -- *.gch *.gcov *.gcno *.gcda *.$(LIBEXT)
	-rm -f -- *.t *.to *.f *.fo

.SUFFIXES:
.SUFFIXES: .lo .o .c .to .t .fo .f

.PHONY: all check fuzz install uninstall clean
