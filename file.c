/* file.c - File and filetree funtions

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
#include "hash.h"

#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static struct ufile * file_add (struct ufile *root, struct ufile *file);
static void ufile_destroy_all (struct ufile *f);


static struct ufile * file_add (struct ufile *root, struct ufile *file)
{
	struct ufile *result;
	int cmp = digest_cmp(file->digest, root->digest);

	if (cmp < 0) {
		if (root->left == NULL) {
			root->left = file;
			file->parent = root;
			result = file;
		} else {
			result = file_add(root->left, file);
		}

	} else if (cmp > 0) {
		if (root->right == NULL) {
			root->right = file;
			file->parent = root;
			result = file;
		} else {
			result = file_add(root->right, file);
		}

	} else {
		/* TODO: add logic for checking if entry will be put to
		 * overflow chain */

		/* ufile already exists, return the existing entry */
		result = root;
	}

	return result;
}


struct ufile * ft_add_file (struct ft *ft, struct ufile *f)
{
	struct ufile *tmp;

	if (ft->root == NULL) {
		ft->root = f;
		tmp = f;
	} else {
		tmp = file_add(ft->root, f);
	}

	return tmp;
}


struct ufile * ft_add_filepath (struct ft *ft, const char *fp)
{
	struct ufile *f = ufile_init(fp);
	struct ufile *tmp = ft_add_file(ft, f);

	if (f != tmp)
		ufile_destroy(f);

	return tmp;
}


void ft_destroy (struct ft *ft)
{
	free(ft);
}


void ft_destroy_all (struct ft *ft)
{
	if (ft->root != NULL)
		ufile_destroy_all(ft->root);
	free(ft);
}


struct ft * ft_init ()
{
	struct ft *ft = malloc(sizeof(struct ft));
	ft->root = NULL;
	return ft;
}


bool get_filetype (const char *filepath, mode_t * restrict type)
{
	bool res = true;
	struct stat sb;

	if (lstat(filepath, &sb) == -1)
		res = false;
	else
		*type = sb.st_mode;

	return res;
}


void ufile_destroy (struct ufile *f)
{
	free(f->filepath);
	free(f->digest);
	free(f);
}


static void ufile_destroy_all (struct ufile *f)
{
	if (f->left != NULL)
		ufile_destroy_all(f->left);

	if (f->right != NULL)
		ufile_destroy_all(f->right);

	ufile_destroy(f);
}


struct ufile * ufile_init (const char *fp)
{
	struct ufile * f = malloc(sizeof(struct ufile));
	f->filepath = strdup(fp);
	f->digest = malloc(DIGEST_LEN);
	hash_file(fp, f->digest); /* TODO: handle possible errors */

	f->parent = NULL;
	f->left = NULL;
	f->right = NULL;

	return f;
}
