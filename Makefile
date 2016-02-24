
include generic.mk

src/%/a.out: src/%
	$(MAKE) -C $< a.out

src/%.o: src/%.c
	$(C) -c $? -o $@

bin/%: src/%.o
	$(C) $? -o $@

bin/%: src/%/a.out
	mv $< $@



