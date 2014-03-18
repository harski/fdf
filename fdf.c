/* fdf - Find duplicate files.

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


#include "file.h"

#include <ftw.h>
#include <stdio.h>
#include <stdlib.h>

#define FTW_FD_CNT 20


struct options {
	struct ft *ft;
} opt;


int handle_file (const char *fpath, const struct stat *sb, int typeflag);
void init_options (struct options *op);


int main (int argc, char **argv)
{
	int success;
	init_options(&opt);

	if (argc < 2) {
		fprintf(stderr, "Error: invalid syntax\n");
		fprintf(stderr, "usage: %s path-to-dir\n", argv[0]);
		return 1;
	}

	success = ftw(argv[1], handle_file, FTW_FD_CNT);

	printf("success: %d\n", success);
	ft_destroy_all(opt.ft);

	return 0;
}


int handle_file (const char *fpath, const struct stat *sb, int typeflag)
{
	struct file *f;
	int retval = 0;

	if (typeflag == FTW_F) {
		f = file_init(fpath);
		ft_add_file(opt.ft, f);

		/* if file is duplicate, find the original */
		if (f->prev != NULL) {
			while (f->prev != NULL)
				f = f->prev;

			printf("%s is duplicate of %s\n", fpath, f->filepath);
		}
	} else if (typeflag == FTW_D) {
		/* skip the dirs */
	} else {
		fprintf(stderr, "Error: something funky happened with '%s'\n",
			fpath);
		fprintf(stderr, "Quitting...\n");
		retval = -1;
	}

	return retval;
}


void init_options (struct options *op)
{
	op->ft = ft_init();
}

