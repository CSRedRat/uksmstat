/*
    uksmstat — small tool to show UKSM statistics
    Copyright © 2012 Oleksandr Natalenko aka post-factum <pfactum@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sysexits.h>

#define UKSMDIR		"/sys/kernel/mm/uksm"
#define UKSMRUN		UKSMDIR"/run"
#define UKSMUNSHARED	UKSMDIR"/pages_unshared"
#define UKSMSHARED	UKSMDIR"/pages_sharing"
#define UKSMSCANNED	UKSMDIR"/pages_scanned"

void show_help()
{
	fprintf(stdout, "uksmstat - small tool to show UKSM statistics\n");
	fprintf(stdout, "© Oleksandr Natalenko aka post-factum, 2012\n");
	fprintf(stdout, "Distributed under terms and conditions of GPLv3+. See COPYING for details.\n");
	fprintf(stdout, "Usage: uksmstat <options>\n");
	fprintf(stdout, "Options:\n");
	fprintf(stdout, "\t-a: show whether UKSM is active\n");
	fprintf(stdout, "\t-u: show unshared memory\n");
	fprintf(stdout, "\t-s: show saved memory\n");
	fprintf(stdout, "\t-c: show scanned memory\n");
	fprintf(stdout, "\t-k: use kibibytes\n");
	fprintf(stdout, "\t-m: use mebibytes\n");
	fprintf(stdout, "\t-g: use gibibytes\n");
	fprintf(stdout, "\t-v: be verbose (up to -vv)\n");
	fprintf(stdout, "\t-h: show this help\n");
	exit(EX_OK);
}

int main(int argc, char **argv)
{
	// define vars
	int opts = 0, active = 0, unshared = 0, shared = 0, scanned = 0, verbose = 0;
	char *units;
	unsigned long divisor = 0;
	struct stat sb;
	FILE *f;

	// check if there's uksm
	if (0 != stat(UKSMDIR, &sb) && S_ISDIR(sb.st_mode))
	{
		fprintf(stderr, "Unable to find uksm interface in %s\n", UKSMDIR);
		exit(EX_OSFILE);
	}

	// parse cmdline options
	while (-1 != (opts = getopt(argc, argv, "ausckmgvh")))
	{
		switch (opts)
		{
			case 'a':
				active = 1;
				break;
			case 'u':
				unshared = 1;
				break;
			case 's':
				shared = 1;
				break;
			case 'c':
				scanned = 1;
				break;
			case 'k':
				units = "KiB";
				divisor = 1024;
				break;
			case 'm':
				units = "MiB";
				divisor = 1024 * 1024;
				break;
			case 'g':
				units = "GiB";
				divisor = 1024 * 1024 * 1024;
				break;
			case 'v':
				verbose++;
				break;
			case 'h':
				show_help();
				break;
			default:
				fprintf(stderr, "Unknown option: %c\n", opts);
				exit(EX_USAGE);
				break;
		}
	}

	if (1 == active)
	{
		f = fopen(UKSMRUN, "r");
		if (NULL == f)
		{
			fprintf(stderr, "Unable to open run file\n");
			exit(EX_OSFILE);
		}
		int run;
		fscanf(f, "%d", &run);
		fclose(f);

		if (1 == run)
			fprintf(stdout, "UKSM is active\n");
		else if (0 == run)
			fprintf(stdout, "UKSM is inactive\n");
	}

	// find out page size
	unsigned int page_size = sysconf(_SC_PAGESIZE);
	if (-1 == page_size)
	{
		fprintf(stderr, "Unable to get page size\n");
		exit(EX_OSERR);
	}

	// show unshared mem
	if (1 == unshared)
	{
		f = fopen(UKSMUNSHARED, "r");
		if (NULL == f)
		{
			fprintf(stderr, "Unable to open pages_unshared file\n");
			exit(EX_OSFILE);
		}
		unsigned long long pages_unshared;
		fscanf(f, "%llu", &pages_unshared);
		fclose(f);

		if (0 == verbose)
			fprintf(stdout, "%1.3lf\n", (double)page_size * pages_unshared / divisor);
		else if (1 == verbose)
			fprintf(stdout, "%1.3lf %s\n", (double)page_size * pages_unshared / divisor, units);
		else if (2 == verbose)
			fprintf(stdout, "Unshared pages: %1.3lf %s\n", (double)page_size * pages_unshared / divisor, units);
	}

	// show shared (saved) mem
	if (1 == shared)
	{
		f = fopen(UKSMSHARED, "r");
		if (NULL == f)
		{
			fprintf(stderr, "Unable to open pages_sharing file\n");
			exit(EX_OSFILE);
		}
		unsigned long long pages_shared;
		fscanf(f, "%llu", &pages_shared);
		fclose(f);
		if (0 == verbose)
			fprintf(stdout, "%1.3lf\n", (double)page_size * pages_shared / divisor);
		else if (1 == verbose)
			fprintf(stdout, "%1.3lf %s\n", (double)page_size * pages_shared / divisor, units);
		else if (2 == verbose)
			fprintf(stdout, "Shared pages: %1.3lf %s\n", (double)page_size * pages_shared / divisor, units);
	}

	// show scanned (total during kernel uptime) mem
	if (1 == scanned)
	{
		f = fopen(UKSMSCANNED, "r");
		if (NULL == f)
		{
			fprintf(stderr, "Unable to open pages_scanned file\n");
			exit(EX_OSFILE);
		}
		unsigned long long pages_scanned;
		fscanf(f, "%llu", &pages_scanned);
		fclose(f);
		if (0 == verbose)
			fprintf(stdout, "%1.3lf\n", (double)page_size * pages_scanned / divisor);
		else if (1 == verbose)
			fprintf(stdout, "%1.3lf %s\n", (double)page_size * pages_scanned / divisor, units);
		else if (2 == verbose)
			fprintf(stdout, "Scanned pages: %1.3lf %s\n", (double)page_size * pages_scanned / divisor, units);
	}

	return EX_OK;
}

