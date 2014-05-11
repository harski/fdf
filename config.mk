# Customize below to suit your system

# fdf version
VERSION = 0.01

# Paths
PREFIX = /usr/local

CC = cc
CFLAGS = -std=c99 -g -Wall -Werror -pedantic -D_XOPEN_SOURCE=500 -DVERSION=\"$(VERSION)\"
LDFLAGS = -lcrypto

