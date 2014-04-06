/* file.h - Interface definitions for file and filetree functions.

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


#ifndef FDF_FILE_H
#define FDF_FILE_H


/* unique file struct */
struct ufile {
	char *filepath;

	/* unique identifier of the file */
	unsigned char *digest;

	/* other files with the same digest */
	struct ufile *next;
	struct ufile *prev;

	/* file tree pointers */
	struct ufile *parent;
	struct ufile *left;
	struct ufile *right;
};


struct ft {
	struct ufile *root;
};


void ufile_destroy (struct ufile *f);
struct ufile *ufile_init (const char *fp);

struct ufile * ft_add_file (struct ft *ft, struct ufile *f);
struct ufile * ft_add_filepath (struct ft *ft, const char *fp);

void ft_destroy (struct ft *ft);
void ft_destroy_all (struct ft *ft);
struct ft *ft_init ();

#endif

