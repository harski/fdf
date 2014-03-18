/* hash.c - Hash functions.

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


#include "hash.h"

#include <openssl/evp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define FILE_BUF_SIZE 1024

static int update_hash (FILE *file, EVP_MD_CTX *mdctx);

int digest_cmp (const unsigned char *d1, const unsigned char *d2)
{
	return strncmp((const char *)d1, (const char *)d2, DIGEST_LEN);
}


int hash_file (const char *filepath, unsigned char *digest)
{
	EVP_MD_CTX *mdctx;
	const EVP_MD *md;
	FILE *file;
	int success;
	unsigned hash_len;

	file = fopen(filepath, "r");
	if (!file) {
		fprintf(stderr,
			"Error: couldn't open file '%s' for reading\n",
			filepath);
		return 0;
	}

	md = EVP_sha1();
	mdctx = EVP_MD_CTX_create();
	EVP_DigestInit_ex(mdctx, md, NULL);

	/* do the reading */
	while (update_hash(file, mdctx))
		;

	/* check why reading stopped failed */
	if (feof(file)) {
		/* the file ended */
		success = 1;
	} else if (ferror(file)) {
		success = 0;
		fprintf(stderr, "Error in reading file '%s'\n", filepath);
	} else {
		success = 0;
		fprintf(stderr, "Error in updating the file '%s' digest\n",
			filepath);
	}

	fclose(file);

	if (success > 0) {
		EVP_DigestFinal_ex(mdctx, digest, &hash_len);
	}

	EVP_MD_CTX_destroy(mdctx);

	return success;
}


static int update_hash (FILE *file, EVP_MD_CTX *mdctx)
{
	int read_ok = 1;
	unsigned char buffer[FILE_BUF_SIZE];
	size_t r_size = fread(buffer, sizeof(char),
				FILE_BUF_SIZE, file);

	if (!r_size) {
		read_ok = 0;
	} else {
		/* feed the data to digest function */
		read_ok = EVP_DigestUpdate(mdctx, buffer, r_size);
	}

	return read_ok;
}

