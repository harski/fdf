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

include config.mk

src = fdf.c file.c hash.c
objs := ${src:.c=.o}

target = fdf

all: $(target) fdf.1

$(target): $(objs)
	$(CC) $(CFLAGS) $(LDFLAGS) $(objs) -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $<

fdf.1: fdf.1.mk
	@sed s/VERSION/$(VERSION)/ < fdf.1.mk > $@

install: $(target)
	install -m 0755 $(target) $(prefix)/bin

uninstall:
	rm $(prefix)/bin/$(target)

clean:
	rm -rf $(objs) $(target) fdf.1

.PHONY: all clean install uninstall
