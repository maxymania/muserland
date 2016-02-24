
include ../../generic.mk

%.o: %.c
	$(C) -c $< -o $@

src += main.o

a.out: $(src)
	$(C) $? -o $@


