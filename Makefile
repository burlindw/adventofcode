INPUT?=%.txt

CFLAGS:=-MMD -g -Og
CPPFLAGS:=-Icommon
LDLIBS:=-lm
LDFLAGS:=

COMMONSRCS:=main.c aoc-array.c
COMMONOBJS:=$(COMMONSRCS:%=.build/common/%.o)
COMMONDEPS:=$(COMMONSRCS:%=.build/common/%.d)

.SECONDEXPANSION:

.PHONY: %/1
%/1: .build/% $(INPUT)
	$^ 1

.PHONY: %/2
%/2: .build/% $(INPUT)
	$^ 2

.PHONY: clean
clean: 
	rf -rf .build

.PRECIOUS: .build/%.c.o
.build/%.c.o: %.c | $$(@D)/
	cc $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

.PRECIOUS: .build/%
.build/%: .build/%.c.o $(COMMONOBJS) | $$(@D)/
	cc $(LDFLAGS) -o $@ $^ $(LDLIBS)

.PRECIOUS: %/
%/:
	mkdir -p $@

-include $(COMMONDEPS)
