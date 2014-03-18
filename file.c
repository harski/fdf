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

static void file_add (struct file *root, struct file *file);
static void file_destroy_all (struct file *f);


void file_add (struct file *root, struct file *file)
{
	int cmp = digest_cmp(file->digest, root->digest);

	if (cmp < 0) {
		if (root->left == NULL) {
			root->left = file;
			file->parent = root;
		} else {
			file_add(root->left, file);
		}

	} else if (cmp > 0) {
		if (root->right == NULL) {
			root->right = file;
			file->parent = root;
		} else {
			file_add(root->right, file);
		}

	} else {
		/* TODO: to save 160 bits per file in the overflow chain link
		 * the digest pointer to the digest in the first file */
		while (root->next != NULL)
			root = root->next;

		root->next = file;
		file->prev = root;
	}
}


void file_destroy (struct file *f)
{
	free(f->filepath);
	free(f->digest);
	free(f);
}


static void file_destroy_all (struct file *f)
{
	if (f->left != NULL)
		file_destroy_all(f->left);

	if (f->right != NULL)
		file_destroy_all(f->right);

	/* destroy overflow chain */
	if (f->next != NULL)
		file_destroy_all(f->next);

	file_destroy(f);
}


struct file * file_init (const char *fp)
{
	struct file * f = malloc(sizeof(struct file));
	f->filepath = strdup(fp);
	f->digest = malloc(DIGEST_LEN);
	hash_file(fp, f->digest); /* TODO: handle possible errors */

	f->prev = NULL;
	f->next = NULL;
	f->parent = NULL;
	f->left = NULL;
	f->right = NULL;

	return f;
}


void ft_add_file (struct ft *ft, struct file *f)
{
	if (ft->root == NULL)
		ft->root = f;
	else
		file_add(ft->root, f);
}


void ft_add_filepath (struct ft *ft, const char *fp)
{
	struct file *f = file_init(fp);
	ft_add_file(ft, f);
}


void ft_destroy (struct ft *ft)
{
	free(ft);
}


void ft_destroy_all (struct ft *ft)
{
	if (ft->root != NULL)
		file_destroy_all(ft->root);
	free(ft);
}


struct ft * ft_init ()
{
	struct ft *ft = malloc(sizeof(struct ft));
	ft->root = NULL;
	return ft;
}

