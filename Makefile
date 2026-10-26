# ABOB — text-mode browser
CC      ?= gcc
XML_INC ?= /usr/include/libxml2
CFLAGS  ?= -Wall -I$(XML_INC)
LDLIBS  ?= -lxml2 -lz -llzma -licui18n -licuuc -licudata -lm -lncurses -lcurl

abob: abob.c request.c html.c manage_url.c start_page.c prompt_mode.c abob.h
	$(CC) $(CFLAGS) -o $@ abob.c $(LDLIBS)

clean:
	rm -f abob

.PHONY: clean
