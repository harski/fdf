/* hash.h - Interface definition of the hash functions.

Copyright (C) 2014 Tuomo Hartikainen <tth@harski.org>.

This file is part of fdf.

fdf is free software: you can redistribute it and/or modify it under the terms
of the GNU General Public License as published by the Free Software Foundation,
either version 3 of the License.

fdf is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
fdf. If not, see <http://www.gnu.org/licenses/>. */


#ifndef FDF_HASH_H
#define FDF_HASH_H

/* sha1 len == 160 bits i.e. 20 bytes */
#define DIGEST_LEN 20


int digest_cmp (const unsigned char *d1, const unsigned char *d2);

/* digest must be preallocated and large enough (use digest_init()) to hold the
 * result. */
int hash_file (const char *filepath, unsigned char *digest);

#endif

