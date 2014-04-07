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

#include <dirent.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>


enum {
	OPT_SUCCESS,
	OPT_EXIT_SUCCESS,
	OPT_EXIT_FAILURE
};

struct options {
	struct ft *ft;
} opt;

int handle_file (const char *fpath);
int is_dir (const char *filepath);
void options_destroy (struct options *opt);
void options_init (struct options *opt);
int parse_options (int argc, char **argv, struct options *opt,
			struct stail_file_head *sfh,
			struct stail_dir_head *sdh);
void print_usage ();
int unpack_dir (const char *dirname,
		struct stail_file_head *files_head,
		struct stail_dir_head *dirs_head);


int main (int argc, char **argv)
{
	int result;

	options_init(&opt);

	/* initialize file list */
	struct stail_file_head files_head =
		STAILQ_HEAD_INITIALIZER(files_head);

	/* initialize dir list */
	struct stail_dir_head dirs_head =
		STAILQ_HEAD_INITIALIZER(dirs_head);

	result = parse_options(argc, argv, &opt, &files_head, &dirs_head);
	if (result!=OPT_SUCCESS)
		return result;

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

		/* handle the next file in queue and free it afterwards */
		STAILQ_REMOVE_HEAD(&files_head, files);
		handle_file(tmp->filepath);
		free(tmp->filepath);
		free(tmp);
	}

	/* clean up the options (ufile filetree is cleaned, too) */
	options_destroy(&opt);

	return 0;
}


int handle_file (const char *fpath)
{
	struct ufile *f, *tmp;
	int retval = 1;

	f = ufile_init(fpath);
	tmp = ft_add_file(opt.ft, f);

	/* if ufile wasn't added (is a duplicate) */
	if (f != tmp) {
		printf("%s is duplicate of %s\n", fpath, tmp->filepath);
		ufile_destroy(f);
	}

	return retval;
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


void options_destroy (struct options *opt)
{
	if (opt->ft != NULL)
		ft_destroy_all(opt->ft);
}


void options_init (struct options *opt)
{
	opt->ft = ft_init();
}


int parse_options (int argc, char **argv, struct options *opt,
			struct stail_file_head *sfh,
			struct stail_dir_head *sdh)
{
	int retval = OPT_SUCCESS;

	opterr = 0; /* don't print getopt errors */

	while (1) {
		int optc;
		static struct option long_options[] = {
			{"help",	no_argument,	0,	'h'},
			{"usage",	no_argument,	0,	'h'},
			{0,		0,		0,	0}
		};

		optc = getopt_long(argc, argv, "h",
				long_options, NULL);

		/* all options handled */
		if (optc == -1)
			break;

		switch (optc) {
		case 'h':
			print_usage();
			retval = OPT_EXIT_SUCCESS;
			break;
		default:
			fprintf(stderr, "Error: unknown option '%c'\n", optc);
			retval = OPT_EXIT_FAILURE;
			break;
		}
	}

	/* if exit flag was not raised handle input files */
	if (retval == OPT_SUCCESS) {
		if (optind >= argc) {
			fprintf(stderr, "Error: no input files\n");
			print_usage();
			retval = OPT_EXIT_FAILURE;
		} else {
			for (; optind < argc; ++optind) {
				/* divide input files to directories and others */
				struct file *tmp = malloc(sizeof(struct file));
				tmp->filepath = strdup(argv[optind]);

				if (is_dir(argv[optind])) {
					STAILQ_INSERT_TAIL(sdh, tmp, files);
				} else {
					STAILQ_INSERT_TAIL(sfh, tmp, files);
				}
			}
		}
	}

	return retval;
}


void print_usage ()
{
	printf("usage: fdf FILE|DIR [FILE|DIR ...]\n");
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


