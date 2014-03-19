# Makefile for fdf
#
# Copyright (C) 2014 Tuomo Hartikainen <tth@harski.org>.
#
# This file is part of fdf.
#
# fdf is free software: you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation, either version 3 of the License.
#
# fdf is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
# A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with
# fdf. If not, see <http://www.gnu.org/licenses/>.

CC=gcc
CFLAGS=-std=c99 -Wall -Werror -pedantic -D_XOPEN_SOURCE=500
libs=-lcrypto
objs=fdf.o file.o hash.o
target=fdf
prefix=/usr/local

all: $(target)

$(target): $(objs)
	$(CC) $(CFLAGS) $(libs) $(objs) -o $(target)

fdf.o: fdf.c file.h
	$(CC) -c $(CFLAGS) -o $@ $<

file.o: file.c file.h hash.o
	$(CC) -c $(CFLAGS) -o $@ $<

hash.o: hash.c hash.h
	$(CC) -c $(CFLAGS) -o $@ $<

install: $(target)
	install -m 0755 $(target) $(prefix)/bin

uninstall:
	rm $(prefix)/bin/$(target)

clean:
	@rm -rf *.o $(target)
