
#include "config.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "cmd.h"
#include "duc.h"


static int info_db(char *file) {

    struct duc_index_report *report;
    int i = 0;

    duc *duc = duc_new();
    if(duc == NULL) {
	fprintf(stderr, "Error creating duc context\n");
	return -1;
    }
    
    printf("Reading %s, available indices:\n", file);
    int r = duc_open(duc, file, DUC_OPEN_RO);
    if(r != DUC_OK) {
	fprintf(stderr, "Error!  %s\n", duc_strerror(duc));
	return -1;
    }

    while(( report = duc_get_report(duc, i)) != NULL) {
	
	char ts[32];
	struct tm *tm = localtime(&report->time_start.tv_sec);
	strftime(ts, sizeof ts, "%Y-%m-%d %H:%M:%S",tm);
	
	char siz[32];
	duc_humanize(report->size_total, siz, sizeof siz);

	printf("  %s %7.7s %s\n", ts, siz, report->path);
	
	duc_index_report_free(report);
	i++;
    }
    
    duc_close(duc);
    duc_del(duc);
    return 0;
}

static int info_main(int argc, char **argv)
{
	char *path_db = NULL;
	char *db_dir = NULL;
	int c;
	
	struct option longopts[] = {
		{ "database",       required_argument, NULL, 'd' },
		{ "dbdir",          required_argument, NULL, 'D' },
		{ NULL }
	};

	while( ( c = getopt_long(argc, argv, "d:D:", longopts, NULL)) != EOF) {

		switch(c) {
			case 'd':
				path_db = optarg;
				break;
			case 'D':
				db_dir = optarg;
				break;
			default:
				return -2;
		}
	}

	
	if (db_dir) {
	    glob_t bunch_of_dbs;
	    char **db_file;
	    size_t n = duc_find_dbs(db_dir, &bunch_of_dbs);
	    printf("Found %zu (%zu) DBs to look at.\n", n, bunch_of_dbs.gl_pathc);
	    int i = 0;
	    for (db_file = bunch_of_dbs.gl_pathv; i < n; db_file++, i++) {
		info_db(*db_file);
	    }
	    exit(1);
	}

	path_db = duc_pick_db_path(path_db);
	info_db(path_db);
	return 0;
}



struct cmd cmd_info = {
	.name = "info",
	.description = "Dump database info",
	.usage = "[options]",
	.help = 
		"  -d, --database=ARG      use database file ARG [~/.duc.db]\n",
	.main = info_main
};


/*
 * End
 */

