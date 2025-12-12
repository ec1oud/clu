/*
   clu - Callsign Looker Upper
   adapted from xlog - GTK+ logging program for amateur radio operators
   Copyright (C) 2001 - 2008 Joop Stakenborg <pg4i@amsat.org>
   Copyright (C) 2020 Andy Stewart <kb1oiq@arrl.net>
   Copyright (C) 2025 Shawn Rutledge <s@ecloud.org>

   clu is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   clu is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with clu.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <locale.h>
#include <string.h>
#include <stdio.h>

#include "dxcc.h"

/* command line options */
static void
parsecommandline(int argc, char* argv[])
{
	int p;

	while ((p = getopt(argc, argv, "hv")) != -1) {
		switch (p) {
		case 'v':
			printf("cty version %d\n", readctyversion());
			exit(0);
		case ':':
		case '?':
		case 'h':
			printf("Usage: clu [option] callsign\n");
			printf("	-h	Display this help and exit\n");
			printf("	-v	Output version information and exit\n");
			exit(0);
		}
	}
}

/* the fun starts here */
int main(int argc, char* argv[])
{
	parsecommandline(argc, argv);
	if (argc < 2)
		return -1;
	readctydata();
#ifdef USE_AREA_DAT
	readareadata();
#endif
	dxcc_data ci = lookupcountry_by_callsign(argv[1]);
#ifdef USE_AREA_DAT
	cleanup_area();
#endif
	cleanup_dxcc();
}
