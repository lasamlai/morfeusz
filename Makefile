NAME=	$(shell swipl -g "name(X),write(X)" -t halt pack.pl)
VERSION=$(shell swipl -g "version(X),write(X)" -t halt pack.pl)
LIBS+=	-lmorfeusz2
SOBJ=	$(PACKSODIR)/morfeusz2_swipl.$(SOEXT)

all: $(SOBJ)

$(PACKSODIR)/%.$(SOEXT): c/%.o
	mkdir -p $(PACKSODIR)
	$(LD) $(ARCH) $(LDSOFLAGS) -o $@ $< $(LIBS) $(SWISOLIB) 
	strip -x $@

c/%.o: c/%.cc
	$(CC) $(ARCH) $(CFLAGS) -c -o $@ $<

install-me:
	swipl -g "pack_install('file:.',[upgrade(true)])" -t halt

remove:
	swipl -g "pack_remove($(NAME))" -t halt

install::
check::
	$(SWIPL) -g "use_module(library(morfeusz))" -t halt
	$(SWIPL) -g "run_tests" -t halt test/test.pl

clean:
	rm -rf *~ ./*/*~ ./*/*.o

distclean: clean
	rm -f $(SOBJ)

package: clean
	tar zcvf "$(NAME)-$(VERSION).tgz" pack.pl prolog c README.md Makefile
