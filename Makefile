CC ?= gcc
CPPFLAGS ?=
CFLAGS ?= -Wall -Wextra -Wpedantic -O2
LDFLAGS ?=
LDLIBS ?= -lm

PROGRAMS = \
	basic \
	cosine \
	linear \
	smoothstep \
	bezier \
	warm_power \
	fire \
	hsv2rgb \
	gaussian \
	cubehelix \
	spectrum

.PHONY: all clean

all: palette $(PROGRAMS)

basic: src/basic.c palette
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)

%: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@ $(LDFLAGS) $(LDLIBS)

palette: src/palette
	cp $< $@

clean:
	rm -f $(PROGRAMS) palette output.bmp
