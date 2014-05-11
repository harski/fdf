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


#include <dirent.h>
#include <error.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "debug.h"
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
};

struct file * get_next_file (struct stail_file_head *files,
				struct stail_file_head *dirs);
int find_duplicates (struct options *opt,
			struct stail_file_head *files_head,
			struct stail_file_head *dirs_head);
int handle_file (const struct options *opt, const char *fpath);
int is_dir (const char *filepath);
void options_destroy (struct options *opt);
struct options * options_init ();
int parse_options (int argc,
			char **argv,
			struct options *opt,
			struct stail_file_head *sfh,
			struct stail_file_head *sdh);
void print_usage ();
int unpack_dir (const char *dirname,
		struct stail_file_head *files_head,
		struct stail_file_head *dirs_head);
int validate_action (int *actions);


int main (int argc, char **argv)
{
	int result;
	int action;

	struct options * opt = options_init();

	/* initialize file list */
	struct stail_file_head files_head =
		STAILQ_HEAD_INITIALIZER(files_head);

	/* initialize dir list */
	struct stail_file_head dirs_head =
		STAILQ_HEAD_INITIALIZER(dirs_head);

	result = parse_options(argc, argv, opt, &files_head, &dirs_head);
	if (result!=OPT_SUCCESS)
		return result;

	action = validate_action(opt->actions);
	if (action == AC_ERR) {
		fprintf(stderr, "Error: more than one action given\n");
		print_usage();
		return 1;
	}

	switch(action) {
	case AC_FDF:
		find_duplicates(opt, &files_head, &dirs_head);
		break;
	case AC_HELP:
		debug_print("%s\n", "debug print");
		print_usage();
		break;
	case AC_VERSION:
		printf("fdf version %s\n", VERSION);
		break;
	}

	/* clean up the options (ufile filetree is cleaned, too) */
	options_destroy(opt);

	return 0;
}


struct file * get_next_file (struct stail_file_head *files_h,
				struct stail_file_head *dirs_h)
{
	struct file *next_file;
	next_file = STAILQ_FIRST(files_h);

	/* if files and dirs are finished give up and return NULL */
	while (next_file == NULL && !STAILQ_EMPTY(dirs_h)) {

		/* if there were no files, take next dir, extract its contents
		 * and return the next file */
		struct file *tmp_dir = STAILQ_FIRST(dirs_h);
		STAILQ_REMOVE_HEAD(dirs_h, files);
		unpack_dir(tmp_dir->filepath, files_h, dirs_h);
		free(tmp_dir->filepath);
		free(tmp_dir);

		next_file = STAILQ_FIRST(files_h);
	}

#ifdef DEBUG
	if (next_file != NULL)
		debug_print("handling '%s\n", next_file->filepath);
	else
		debug_print("%s\n", "no next_file to handle");
#endif

	return next_file;
}


int find_duplicates (struct options *opt,
			struct stail_file_head *files_head,
			struct stail_file_head *dirs_head)
{
	struct file *tmp;
	while (1) {
		/* if files and dirs are finished return */
		tmp = get_next_file(files_head, dirs_head);
		if (tmp == NULL)
			break;

		/* handle the next file in queue and free it afterwards */
		STAILQ_REMOVE_HEAD(files_head, files);
		handle_file(opt, tmp->filepath);
		free(tmp->filepath);
		free(tmp);
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

	free(opt);
}


struct options * options_init ()
{
	struct options *opt = malloc(sizeof(struct options));
	opt->ft = ft_init();

	for (int i = 0; i < AC_CNT; ++i)
		opt->actions[i] = 0;

	return opt;
}


int parse_options (int argc, char **argv, struct options *opt,
			struct stail_file_head *sfh,
			struct stail_file_head *sdh)
{
	int retval = OPT_SUCCESS;
	opterr = 0; /* don't print getopt errors */

	while (1) {
		int optc;
		static struct option long_options[] = {
			{"help",	no_argument,	0,	'h'},
			{"usage",	no_argument,	0,	'h'},
			{"version",	no_argument,	0,	'V'},
			{0,		0,		0,	0}
		};

		optc = getopt_long(argc, argv, "hV",
				long_options, NULL);

		/* all options handled */
		if (optc == -1)
			break;

		switch (optc) {
		case 'h':
			opt->actions[AC_HELP] = 1;
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

	return retval;
}


void print_usage ()
{
	printf("usage: fdf (ACTION | [OPTION]... (FILE|DIR)....)\n");
	printf("\nACTIONS:\n");
	printf("-h, --help, --usage\tPrint this help.\n");
	printf("-V, --version\tPrint version information.\n");
}


int unpack_dir (const char *dirname,
		struct stail_file_head *files_head,
		struct stail_file_head *dirs_head)
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
