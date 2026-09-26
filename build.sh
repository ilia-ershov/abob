#!/bin/sh

set -eu

cc -o abob abob.c \
	-I/opt/local/include/libxml2 \
	-lxml2 -lncurses -lcurl
