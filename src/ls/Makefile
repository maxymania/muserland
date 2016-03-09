
include ../../generic.mk

%.o: %.c
	$(C) -c $< -o $@

src += main.o
src += lslib.o

a.out: $(src)
	$(L) $? -o $@


