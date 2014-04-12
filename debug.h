/* Copyright 2013 Tuomo Hartikainen <tuomo.hartikainen@harski.org>.
 * Licensed under the 2-clause BSD license, see LICENSE for details. */

#ifndef H_DEBUG_H
#define H_DEBUG_H

#ifndef DEBUG
	#define DEBUG 0
#endif

#define debug_print(fmt, ...) \
	do { if (DEBUG) fprintf(stderr, "%s:%d:%s(): " fmt, __FILE__, \
				__LINE__, __func__, __VA_ARGS__); } while (0)
#endif

