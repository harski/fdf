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

#ifndef VERSION
#define VERSION "unknown"
#endif

#include <dirent.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "file.h"

enum {
	OPT_SUCCESS,
	OPT_EXIT_SUCCESS,
	OPT_EXIT_FAILURE
};

enum ACTIONS {
	AC_FDF,
	AC_HELP,
	AC_VERSION,
	AC_CNT,
	AC_ERR
};

struct options {
	struct ft *ft;
	int actions[AC_CNT]; /* all the available actions */

	/* how deep to go in directory tree. argument files and dirs are on
	 * level 0, and negative value means there is no limit */
	int dir_tree_depth;

	/* should symlinks be ignored or handled as if they were the file they
	 * are linking to */
	bool handle_symlinks;
};

struct file * get_next_file (const struct options *opt,
				struct stail_file_head *files,
				struct stail_file_head *dirs);
int find_duplicates (const struct options *opt,
			struct stail_file_head *files_head,
			struct stail_file_head *dirs_head);
int handle_file (const struct options *opt, const char *fpath);
void options_destroy (struct options *opt);
struct options * options_init ();
int parse_options (int argc,
			char **argv,
			struct options *opt,
			struct stail_file_head *sfh);
static inline int parse_uint (const char *str, int *i);
void print_help ();
static inline void remove_trailing_slash(char *str);
int unpack_dir (const struct file *dir_f,
		struct stail_file_head *files_head);
int validate_action (int *actions);


int main (int argc, char **argv)
{
	int result;
	int action;

	struct options * opt = options_init();

	/* initialize file input list */
	struct stail_file_head files_head =
		STAILQ_HEAD_INITIALIZER(files_head);

	/* initialize dir list */
	struct stail_file_head dirs_head =
		STAILQ_HEAD_INITIALIZER(dirs_head);

	result = parse_options(argc, argv, opt, &files_head);
	if (result!=OPT_SUCCESS)
		return result;

	action = validate_action(opt->actions);
	if (action == AC_ERR) {
		fprintf(stderr, "Error: more than one action given\n");
		print_help();
		return 1;
	}

	switch(action) {
	case AC_FDF:
		if (STAILQ_EMPTY(&files_head) && STAILQ_EMPTY(&dirs_head)) {
			fprintf(stderr, "Error: no input.\n\n");
			print_help();
			return 2;
		}
		find_duplicates(opt, &files_head, &dirs_head);
		break;
	case AC_HELP:
		print_help();
		break;
	case AC_VERSION:
		printf("fdf version %s\n", VERSION);
		break;
	}

	/* clean up the options (ufile filetree is cleaned, too) */
	options_destroy(opt);

	return 0;
}


/* returns the next file \DIR in the IFQ. If no files are present the next
 * directory in directory queue is expanded and the next file is looked for
 * again. If there is no next file NULL is returned */
struct file * get_next_file (const struct options *opt,
				struct stail_file_head *files_h,
				struct stail_file_head *dirs_h)
{
	struct file *next_file;

	do {
		if (!STAILQ_EMPTY(files_h)) {
			next_file = STAILQ_FIRST(files_h);
			STAILQ_REMOVE_HEAD(files_h, files);
		} else if (!STAILQ_EMPTY(dirs_h)) {
			next_file = STAILQ_FIRST(dirs_h);
			STAILQ_REMOVE_HEAD(dirs_h, files);
		} else {
			/* All input queues are empty, bail out */
			next_file = NULL;
			break;
		}

		/* if next_file is a directory unpack and free it */
		if (S_ISDIR(next_file->type)) {
			if (opt->dir_tree_depth == -1
				|| opt->dir_tree_depth > next_file->depth) {
				unpack_dir(next_file, files_h);
			}

			file_destroy(next_file);
			next_file = NULL;
		}

	} while (!next_file);


	return next_file;
}


int find_duplicates (const struct options *opt,
			struct stail_file_head *files_head,
			struct stail_file_head *dirs_head)
{
	struct file *nfile;

	/* while there are files to go through... */
	while ((nfile = get_next_file(opt, files_head, dirs_head)) != NULL) {

		if (S_ISREG(nfile->type)) {
			/* regular file, handle it normally */
			handle_file(opt, nfile->filepath);
		} else if (S_ISLNK(nfile->type)) {
			/* check from options if symlinks should be handled */
			if (opt->handle_symlinks) {
				handle_file(opt, nfile->filepath);
			} else {
				fprintf(stderr,
					"'%s' is a symlink: ignoring.\n",
					nfile->filepath);
			}
		} else {
			/* unfamiliar file type: ignore the file and print out
			 * an error */
			fprintf(stderr,
				"'%s' is not a regular file: ignoring.\n",
				nfile->filepath);
		}

		/* the file is handled, free it */
		free(nfile->filepath);
		free(nfile);
	}

	return 1;
}


int handle_file (const struct options *opt, const char *fpath)
{
	struct ufile *f, *tmp;
	int retval = 1;

	f = ufile_init(fpath);
	tmp = ft_add_file(opt->ft, f);

	/* if ufile wasn't added (is a duplicate) */
	if (f != tmp) {
		printf("%s is duplicate of %s\n", fpath, tmp->filepath);
		ufile_destroy(f);
	}

	return retval;
}


void options_destroy (struct options *opt)
{
	if (opt->ft != NULL)
		ft_destroy_all(opt->ft);

	free(opt);
}


struct options * options_init ()
{
	struct options *opt = malloc(sizeof(struct options));
	opt->ft = ft_init();

	for (int i = 0; i < AC_CNT; ++i)
		opt->actions[i] = 0;

	opt->handle_symlinks = false;
	opt->dir_tree_depth = -1;

	return opt;
}


int parse_options (int argc, char **argv, struct options *opt,
			struct stail_file_head *sfh)
{
	int retval = OPT_SUCCESS;
	opterr = 0; /* don't print getopt errors */

	while (1) {
		int optc;
		static struct option long_options[] = {
			{"tree-depth",	required_argument,	0,	'd'},
			{"help",	no_argument,	0,	'h'},
			{"include-symlinks",	no_argument,	0,	's'},
			{"version",	no_argument,	0,	'V'},
			{0,		0,		0,	0}
		};

		optc = getopt_long(argc, argv, "d:hVs",
				long_options, NULL);

		/* all options handled */
		if (optc == -1)
			break;

		switch (optc) {
		case 'd':
			if (optind != -1 &&
				!parse_uint(optarg, &(opt->dir_tree_depth))) {
				fprintf(stderr,
					"Error: invalid tree depth '%s'\n",
					optarg);
				retval = OPT_EXIT_FAILURE;
			}
			break;
		case 'h':
			opt->actions[AC_HELP] = 1;
			break;
		case 's':
			opt->handle_symlinks = true;
			break;
		case 'V':
			opt->actions[AC_VERSION] = 1;
			break;
		default:
			fprintf(stderr, "Error: unknown option '%s'\n",
				argv[optind-1]);
			retval = OPT_EXIT_FAILURE;
			break;
		}
	}

	/* if exit flag was not raised handle input files */
	if (retval == OPT_SUCCESS) {
		for (; optind < argc; ++optind) {
			struct file *infile;

			remove_trailing_slash(argv[optind]);

			/* insert filepaths given as arguments to file input
			 * queue */
			infile = file_init_depth(argv[optind], 0);
			if (infile != NULL) {
				STAILQ_INSERT_TAIL(sfh, infile, files);
			} else {
				fprintf(stderr,
					"Could not read file '%s', skipping\n",
					argv[optind]);
			}
		}
	}

	return retval;
}


void remove_trailing_slash(char *str)
{
	int len = strlen(str);
	if (str[len-1] == '/')
		str[len-1] = '\0';
}


static inline int parse_uint (const char *str, int *i)
{
	int retval = 0;
	char *endptr;
	long val = strtol(str, &endptr, 10);

	/* if string is valid */
	if (*str != '\0' && (*endptr) == '\0') {
		*i = (int) val;
		retval = 1;
	}

	return retval;
}


void print_help ()
{
	printf("usage: fdf (ACTION | [OPTION]... (FILE|DIR)....)\n");
	printf("\nACTIONS:\n");
	printf("-h, --help\t\t\tPrint this help.\n");
	printf("-d NUM, --tree-depth NUM\t"
		"Search filetrees up to NUM depth.\n");
	printf("-s, --include-symlinks\t\t"
		"Handle symlinks as if they were the file they are linking to. "
		"The default is to ignore symlinks.\n");
	printf("-V, --version\t\t\tPrint version information.\n");
}


int unpack_dir (const struct file *dir_f,
		struct stail_file_head *files_head)
{
	size_t dirname_len = strlen(dir_f->filepath);
	struct dirent *files;
	DIR *dir = opendir(dir_f->filepath);

	if (dir==NULL) {
		perror(NULL);
		return 0;
	}

	while ((files = readdir(dir)) != NULL) {
		int depth;
		struct file *file;
		char *fname;

		if (!strcmp(files->d_name, ".") || !strcmp(files->d_name, ".."))
			continue;

		/* get full filename */
		fname = malloc(dirname_len + strlen(files->d_name) + 2);
		sprintf(fname, "%s/%s", dir_f->filepath, files->d_name);

		depth = dir_f->depth == -1 ? -1 : dir_f->depth + 1;

		/* create the file struct, with file depth one more than */
		file = file_init_depth(fname, depth);

		if (file != NULL) {
			/* insert to file input queue */
			STAILQ_INSERT_TAIL(files_head, file, files);
		} else {
			fprintf(stderr,
				"file '%s' could not be read: skipping\n",
				fname);
		}

		free(fname);
	}

	closedir(dir);
	return 1;
}


int validate_action (int *actions)
{
	int retval = AC_FDF; /* init with default action */
	int ac_cnt = 0;
	int i;

	for (i=0; i<AC_CNT; ++i) {
		if (actions[i]) {
			retval = i;
			++ac_cnt;
		}
	}

	/* if no action specified, do fdf */
	if (ac_cnt > 1)
		retval = AC_ERR;

	return retval;
}
