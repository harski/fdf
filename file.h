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

#include <sys/queue.h>

struct file {
	char * filepath;
	STAILQ_ENTRY(file) files;
};

STAILQ_HEAD(stail_file_head, file);
STAILQ_HEAD(stail_dir_head, file);


/* unique file struct */
struct ufile {
	char *filepath;

	/* unique identifier of the file */
	unsigned char *digest;

	/* file tree pointers */
	struct ufile *parent;
	struct ufile *left;
	struct ufile *right;
};


/* filetree struct */
struct ft {
	struct ufile *root;
};


struct ufile *ufile_init (const char *fp);
void ufile_destroy (struct ufile *f);


struct ft *ft_init ();
void ft_destroy (struct ft *ft);

/* free struct ft and the tree below it */
void ft_destroy_all (struct ft *ft);

/* add ufile to the filetree */
struct ufile * ft_add_file (struct ft *ft, struct ufile *f);

/* create the ufile based on the the filepath and add it to the filetree */
struct ufile * ft_add_filepath (struct ft *ft, const char *fp);

#endif

