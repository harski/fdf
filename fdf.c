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

#include <assert.h>
#include <dirent.h>
#include <error.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <sys/types.h>

#define FTW_FD_CNT 20


struct options {
	struct ft *ft;
} opt;

struct file {
	char * filepath;
	STAILQ_ENTRY(file) files;
};

STAILQ_HEAD(stail_file_head, file);
STAILQ_HEAD(stail_dir_head, file);

int handle_file (const char *fpath);
void init_options (struct options *op);
int is_dir (const char *filepath);
void print_usage ();
int unpack_dir (const char *dirname,
		struct stail_file_head *files_head,
		struct stail_dir_head *dirs_head);


int main (int argc, char **argv)
{
	init_options(&opt);

	if (argc < 2) {
		fprintf(stderr, "Error: invalid syntax\n");
		print_usage();
		return 1;
	}

	/* initialize file list */
	struct stail_file_head files_head =
		STAILQ_HEAD_INITIALIZER(files_head);
	STAILQ_INIT(&files_head);

	/* initialize dir list */
	struct stail_dir_head dirs_head =
		STAILQ_HEAD_INITIALIZER(dirs_head);
	STAILQ_INIT(&dirs_head);

	/* divide input files to directories and others */
	for (int i = 1; i<argc; ++i) {
		struct file *tmp = malloc(sizeof(struct file));
		tmp->filepath = strdup(argv[i]);

		if (is_dir(argv[i])) {
			STAILQ_INSERT_TAIL(&dirs_head, tmp, files);
		} else {
			STAILQ_INSERT_TAIL(&files_head, tmp, files);
		}
	}

	struct file *tmp;
	while (1) {
		tmp = STAILQ_FIRST(&files_head);

		if (tmp == NULL) {
			/* if files and dirs are finished, break */
			if (STAILQ_EMPTY(&dirs_head)) {
				break;
			} else {
				/* take the dir off from the list,
				 * pass it to the function and free it */
				struct file *tmp_dir = STAILQ_FIRST(&dirs_head);
				STAILQ_REMOVE_HEAD(&dirs_head, files);
				unpack_dir(tmp_dir->filepath, &files_head, &dirs_head);
				free(tmp_dir->filepath);
				free(tmp_dir);
				continue;
			}
		}

		STAILQ_REMOVE_HEAD(&files_head, files);
		handle_file(tmp->filepath);
		free(tmp->filepath);
		free(tmp);
	}

	return 0;
}


int unpack_dir (const char *dirname,
		struct stail_file_head *files_head,
		struct stail_dir_head *dirs_head)
{
	size_t dirname_len = strlen(dirname);
	struct dirent *files;
	DIR *dir = opendir(dirname);

	if (dir==NULL) {
		perror(NULL);
		return 0;
	}

	while ((files = readdir(dir)) != NULL) {
		struct file *iftmp;

		if (!strcmp(files->d_name, ".") || !strcmp(files->d_name, ".."))
			continue;

		/* create the file struct */
		iftmp = malloc(sizeof(struct file));
		iftmp->filepath = malloc(dirname_len + strlen(files->d_name) + 2);
		sprintf(iftmp->filepath, "%s/%s", dirname, files->d_name);

		/* insert to queue */
		if (is_dir(iftmp->filepath)) {
			STAILQ_INSERT_TAIL(dirs_head, iftmp, files);
		} else {
			STAILQ_INSERT_TAIL(files_head, iftmp, files);
		}
	}

	closedir(dir);
	return 1;

}

int handle_file (const char *fpath)
{
	struct ufile *f;
	int retval = 0;

	f = ufile_init(fpath);
	ft_add_file(opt.ft, f);

	/* if file is duplicate, find the original */
	if (f->prev != NULL) {
		while (f->prev != NULL)
			f = f->prev;

		printf("%s is duplicate of %s\n", fpath, f->filepath);
	}

	return retval;
}


void init_options (struct options *op)
{
	op->ft = ft_init();
}


int is_dir (const char *filepath)
{
	int isdir;
	struct stat sb;

	if (stat(filepath, &sb) == -1) {
		isdir = -1;
	} else if (S_ISDIR(sb.st_mode)) {
		isdir = 1;
	} else {
		isdir = 0;
	}

	return isdir;
}


void print_usage ()
{
	printf("usage: fdf FILE|DIR [FILE|DIR ...]\n");
}

